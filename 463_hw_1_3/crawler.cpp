/*
	Author: Ke Wang
	UIN: 822002009
*/

#include "stdafx.h"
#include "crawler.h"
#include <memory>
#include <iostream>
#include <fstream>


void Crawler::parse_url(string url,ParsedLink* pl)
{
	if(url.find("http://") != std::string::npos)
	{
		url = url.substr(url.find("//") + 2);
	}
	if(url.find(":") != std::string::npos)
	{
		pl->host = url.substr(0,url.find(":"));
		if(url.find("/") != std::string::npos)
		{	
			pl->port = atoi(url.substr(url.find(":") + 1,url.find("/") -1 - url.find(":")).c_str());
			pl->request = url.substr(url.find("/"));
		}
		else
		{	
			pl->port = atoi(url.substr(url.find(":") + 1).c_str());
			pl->request = "/";
		}
	}
	else
	{
		if(url.find("/") != std::string::npos)
		{	
			pl->host = url.substr(0,url.find("/"));
			pl->port = 80;
			pl->request = pl->request = url.substr(url.find("/"));
		}
		else
		{
			pl->host = url;
			pl->port = 80;
			pl->request = "/";
		}
	}
}


bool Crawler::is_dup(string redirect_ulr)
{
	bool ret;
	EnterCriticalSection(&set_lock);
	if(set.find(redirect_ulr) == set.end())
		ret = false;
	else ret = true;
	LeaveCriticalSection(&set_lock);
	return ret;

}

bool Crawler::is_secure(string redirect_ulr)
{
	return redirect_ulr.find("https") == string::npos ? false: true;
}


DWORD Crawler::Crawl() 
{ 
	CrawlerObject co; 
	HTTP http;
	Url url;
	if(seed.find("http:") != string::npos)
		url.pl.host = seed.substr(seed.find_last_of("/")+1);
	else
		url.pl.host = seed;
	url.pl.port = 80;
	url.pl.request = "/";
	url.redirectCount = 0;
	cc.InsertNewURL(&url);

	HTMLparserBase* hp = SpawnNewHTMLParser();
	
	long start_time = GetTickCount()/1000;
	while (true) 
	{   
		EnterCriticalSection(&static_lock);
		statistic.elapse_time = GetTickCount()/1000 - start_time;
		statistic.pending_ip = cc.heap.size();
		statistic.Total_Ip = cc.ipMap.size();
		statistic.pending_links = cc.pending_links;
		LeaveCriticalSection(&static_lock);

		if (co.ipr != NULL)
		{          
			cc.ReinsertHeap (co.ipr);  			
		}

		cc.RemoveHeap (&co);  
		ParsedLink qe = co.url.pl;
		auto it = co.h->rpMap.find(qe.port); 
		RobotParserBase *rp = NULL;
		
		if (it == co.h->rpMap.end())   
		{    
			if(thread_num/2 ==1)
			{
			    EnterCriticalSection(&static_lock);
				cout<<"[   "<<statistic.elapse_time<<"] --> "<<co.url.pl.host<<":"<<co.url.pl.port<<co.url.pl.request<<" (C "<<statistic.crawled_non_robot_links<<" H "<<statistic.resolved_host;
                cout<<" R "<<statistic.valid_robot<<"/"<<statistic.total_robot_attempt<<" IP "<<cc.heap.size()<<"/"<<cc.ipMap.size()<<")\n";
			    LeaveCriticalSection(&static_lock);
			}
			EnterCriticalSection(&static_lock);
			statistic.total_robot_attempt++;
			LeaveCriticalSection(&static_lock);

			ParsedLink robot;
			robot.host = co.url.pl.host;
			robot.port = co.url.pl.port;
			robot.request = "/robots.txt";

			if(http.download (robot) == ERROR_FAIL)  
			{
				co.h->status = BLACKLISTED;     
				//continue; 
			}

			if(http.parse_header() == ERROR_FAIL)
			{    
				co.h->status = BLACKLISTED;     
				//continue; 
			} 

			if(http.get_body_size() > 16*1000 )
			{
				co.h->status = BLACKLISTED;     
				//continue; 
			}

			if(http.get_status() != 200)
			{
				co.h->status = BLACKLISTED;     
				//continue;  
			}

			if (http.get_status() == 200)     
			{  
				rp = SpawnNewRobotParser(); 
				bool ret = rp->AddRobotData ((char*)http.get_body_start(), http.get_body_size(), "Crawler-ke-wang-1.2"); /*? robot url request part*/
				if (ret == false)  
				{ 
					delete rp; 
					rp = NULL; 
					co.h->status = BLACKLISTED;  
				} 
				
				else if (rp->RecordsFound () == 0) 
				{ 
					delete rp;  
					rp = NULL;    
				} 
				co.h->rpMap [co.url.pl.port] = rp;  
				EnterCriticalSection(&static_lock);
				statistic.valid_robot++;
				LeaveCriticalSection(&static_lock);
			}
		}   
		else    
		{
		    rp = it->second;  
		}

		if (rp != NULL) 
		{  
			if(rp->GetCrawlDelay() != 0)
			{	
				EnterCriticalSection(&static_lock);
				statistic.none_z_delay_robots.push_back(rp->GetCrawlDelay());
				LeaveCriticalSection(&static_lock);
			}

			if (rp->VerifyAllowed ((char*)qe.request.c_str()) == false)   
			{
				continue;
			}
		}

		/*pass robot or none-exist, crawl url*/
		if(qe.request == "/robots.txt")
			continue;
		
		EnterCriticalSection(&static_lock);
		statistic.crawled_non_robot_links++;
		LeaveCriticalSection(&static_lock);
		
		if(thread_num/2 == 1)
		{
		    EnterCriticalSection(&static_lock);
			cout<<"[   "<<statistic.elapse_time<<"]     "<<co.url.pl.host<<":"<<co.url.pl.port<<co.url.pl.request<<" (C "<<statistic.crawled_non_robot_links<<" H "<<statistic.resolved_host;
            cout<<" R "<<statistic.valid_robot<<"/"<<statistic.total_robot_attempt<<" IP "<<cc.heap.size()<<"/"<<cc.ipMap.size()<<")\n";
		    LeaveCriticalSection(&static_lock);
		}
		if(http.download(qe) == ERROR_FAIL)
		{
			continue;
		}
		EnterCriticalSection(&static_lock);
		statistic.download_size += http.get_full_size();
		statistic.page_sizes.push_back(http.get_full_size());
		LeaveCriticalSection(&static_lock);

		if(http.parse_header() == ERROR_FAIL)
		{
			continue;
		}
		if(thread_num == 1)
		    cout<<"    *** status | "<<http.get_status()<<" \n";
		string redirect_ulr;
		string action;
		std::unique_ptr<char> parent_url(new char[1024]);
		
		int status = http.get_status();
		EnterCriticalSection(&static_lock);
		statistic.HTTP_codes.push_back(status);
		LeaveCriticalSection(&static_lock);

		switch(status)
		{
		case 200:
			sprintf(parent_url.get(),"http://%s:%d%s",qe.host.c_str(),qe.port,qe.request.c_str());
			if(!parse_links(hp,(char*)http.get_body_start(),http.get_body_size(),parent_url.get()))
				break;
			break;
		case 301:
		case 302:
			try
			{
				redirect_ulr = http.get_redirect_url();
			}
			catch(...)
			{
				break;
			}

			if(is_secure(redirect_ulr))
			{
				action = "(ignore)";
			}
			else if(is_dup(redirect_ulr))
			{   
				action = "(dup)";
			}
			else
			{	
				action = "(follow)";
				ParsedLink pl;
				parse_url(redirect_ulr,&pl);
				Url url;
				url.pl.host = pl.host;
				url.pl.port = pl.port;
				url.pl.request = pl.request;
				url.redirectCount = co.url.redirectCount + 1;
				if(url.redirectCount > 5)
					break;
				else
				{
				    unique_ptr<char> purl(new char[MAX_VALID_URL]);				
					sprintf(purl.get(),"%s:%d%s",url.pl.host.c_str(),url.pl.port,url.pl.request.c_str());
		            string s;
					s.clear();
		            s = purl.get();
					EnterCriticalSection(&set_lock);
					set.insert(s);
                    cc.InsertNewURL(&url);
					LeaveCriticalSection(&set_lock);
				}
			}
			/*if(thread_num/2 == 1)
				cout<<"redirect "<<redirect_ulr<<" "<<action<<"\n";*/
			break;
		default:

			//if(thread_num/2 == 1)
			//    cout<<"(ignore)\n";
			break;
		}    
	}

   return 0;
}

bool Crawler::parse_links(HTMLparserBase* hp, char *buf, int len, char *parentURL)
{
	queue<ParsedLink>* qpl = hp->Parse(buf, len, parentURL);
	if(!qpl)
	{
		cout<<"Invalid parent url.\n";
		return false;
	}
	ParsedLink pl;
	Url qe;
	string s;
	std::unique_ptr<char> url(new char[MAX_VALID_URL]);
	while(!qpl->empty())
	{
		EnterCriticalSection(&static_lock);
		statistic.parsed_out_links++;
		LeaveCriticalSection(&static_lock);
		pl = qpl->front();
		qpl->pop();
		memset(url.get(),0,MAX_VALID_URL);
		sprintf(url.get(),"%s:%d%s",pl.host.c_str(),pl.port,pl.request.c_str());
		s.clear();
		s = url.get();

		EnterCriticalSection(&set_lock);
		if(set.find(s) == set.end())
		{
			set.insert(s);
			LeaveCriticalSection(&set_lock);
			qe.redirectCount = 0;
			qe.pl = pl;
			cc.InsertNewURL(&qe);
		}
		else
		{
		    LeaveCriticalSection(&set_lock);
		}
	}
	return true;
}

void CrawlerCache::InsertNewURL(Url* link)
{
	
	EnterCriticalSection(&hostmap_lock);
	auto iter = hostMap.find(link->pl.host);
	LeaveCriticalSection(&hostmap_lock);
	
	if(iter == hostMap.end())
	{	
		shared_ptr<HostRecord> host_rec(new HostRecord());
		host_rec->hostname = link->pl.host;
		host_rec->status = DNS_PENDING;
		host_rec->pendingLinks.push_back(*link);
		pending_links++;
		EnterCriticalSection(&hostmap_lock);
		hostMap[link->pl.host] = host_rec;
		LeaveCriticalSection(&hostmap_lock);
		/*send to dns*/	
		EnterCriticalSection(&q_lock);
		QpendingDNS.push(host_rec);
		ReleaseSemaphore(sema,1,NULL);
		LeaveCriticalSection(&q_lock);
	}
	else
	{
		shared_ptr<HostRecord> host_rec(iter->second);
		if(host_rec->status == DNS_FAIL || host_rec->status == BLACKLISTED)
		{		
			return;
		}
		else if(host_rec->status == DNS_EXISTS)
		{
			host_rec->pendingLinks.push_back(*link);
			pending_links++;
			if(host_rec->pendingLinks.size() == 1)
			{
				EnterCriticalSection(&ipmap_lock);
				auto iter = ipMap.find(host_rec->IP);
				LeaveCriticalSection(&ipmap_lock);

				if(iter == ipMap.end())
					return;  
				shared_ptr<IPRecord> ipr = iter->second;
				ipr->pendingHosts.push_back(host_rec);
				if(ipr->status == IDLE)
				{
					ipr->timestamp = GetTickCount()/1000 + 10;
					ipr->status = PENDING;
		            EnterCriticalSection(&heap_lock);
					heap.push(ipr);
					LeaveCriticalSection(&heap_lock);
				}
			}
		}		
	}
	return;	
}

shared_ptr<HostRecord> CrawlerCache::RemoveHostPendingDNS()
{
	WaitForSingleObject(sema,INFINITE);
	EnterCriticalSection(&q_lock);
	shared_ptr<HostRecord> host_record(QpendingDNS.front());
	QpendingDNS.pop();
	LeaveCriticalSection(&q_lock);
	return host_record;
}

void CrawlerCache::AddHostToIP(DWORD IP, shared_ptr<HostRecord>& h)
{
	EnterCriticalSection(&ipmap_lock);
	auto iter = ipMap.find(IP);
	LeaveCriticalSection(&ipmap_lock);
	
	if(iter == ipMap.end())
	{
		shared_ptr<IPRecord> ip_rec(new IPRecord());
		ip_rec->ip = IP;
		ip_rec->status = PENDING;

		EnterCriticalSection(&ipmap_lock);
		ipMap[IP] = ip_rec;
		ip_rec->pendingHosts.push_back(h);
		LeaveCriticalSection(&ipmap_lock);

		long time_stamp = GetTickCount();
		ip_rec->timestamp = time_stamp/1000;/*? million second*/
		
		/*send to heap*/
		EnterCriticalSection(&heap_lock);
		heap.push(ip_rec);/*? what to protect heap*/
		LeaveCriticalSection(&heap_lock);
	}
	else
	{
		shared_ptr<IPRecord> ip_rec(iter->second);

		EnterCriticalSection(&ipmap_lock);
		ip_rec->pendingHosts.push_back(h);/*? what if being crawling*/
		LeaveCriticalSection(&ipmap_lock);

		if(ip_rec->status == IDLE)
		{
			long time_stamp = GetTickCount();
		    ip_rec->timestamp = time_stamp/1000;/*? million second*/
			ip_rec->status = PENDING;
			EnterCriticalSection(&heap_lock);
			heap.push(ip_rec);
			LeaveCriticalSection(&heap_lock);
		}
	}
}

void CrawlerCache::RemoveHeap(CrawlerObject *co)
{
	shared_ptr<IPRecord> ip_rec;
	
	while(1)
	{
		Sleep(100);

		EnterCriticalSection(&heap_lock);
		if(heap.empty())/*heap operation should atomic*/
		{
			LeaveCriticalSection(&heap_lock);
			continue;	
		}
		ip_rec = heap.top();
		long time = GetTickCount();
		if(ip_rec->timestamp > time)
		{
		    LeaveCriticalSection(&heap_lock);
			continue;
		}
		heap.pop();
		LeaveCriticalSection(&heap_lock);

		while(1)
		{
			if(ip_rec->pendingHosts.size() == 0)
				break;

			shared_ptr<HostRecord> hr = ip_rec->pendingHosts.front();
			ip_rec->pendingHosts.pop_front();

			if(hr->pendingLinks.size() == 1)
			{
				co->h = hr;
				co->ipr = ip_rec;
				co->url = hr->pendingLinks.front();
				hr->pendingLinks.pop_front();
				ip_rec->status = CRAWLED;
				return;
			}
			if(hr->pendingLinks.size() > 1)
			{
				co->h = hr;
				co->ipr = ip_rec;
				co->url = hr->pendingLinks.front();
				hr->pendingLinks.pop_front();
				ip_rec->pendingHosts.push_back(hr);
				ip_rec->status = CRAWLED;
				return;
			}	
		}
		/*cant find a url*/
		ip_rec->status = IDLE;	
	}
}

void CrawlerCache::ReinsertHeap(shared_ptr<IPRecord>& ipr)
{
	if(!ipr)
		return;
	if(ipr->pendingHosts.size() != 0)
	{	
		ipr->timestamp = GetTickCount()/1000 + 10;

		ipr->status = PENDING;
		EnterCriticalSection(&heap_lock);
		heap.push(ipr);
		LeaveCriticalSection(&heap_lock);
	}
	else
	{
		ipr->status = IDLE;
	}
}


DWORD Crawler::Resolve()
{
	/*get from to cralwer*/
	while(1)
	{
		shared_ptr<HostRecord> host_rec(cc.RemoveHostPendingDNS());
		
		DWORD address;
		string host_name = host_rec->hostname;

		if(get_address((char*)host_name.c_str(),&address) == ERROR_FAIL)
		{
			host_rec->pendingLinks.clear();
			host_rec->status = DNS_FAIL;
		}
		else
		{
			EnterCriticalSection(&static_lock);
			statistic.resolved_host++;
			statistic.Total_Ip++;
			LeaveCriticalSection(&static_lock);

			host_rec->status = DNS_EXISTS;
			host_rec->IP = address;/*? network order*/	
		}

		/*send to ip*/
		cc.AddHostToIP(host_rec->IP,host_rec);
	}
	return 0;
}

DWORD WINAPI resolve(void* param)
{
	Crawler* c = (Crawler*)param;
	c->Resolve();
	return 0;
}

DWORD WINAPI crawl(void* param)
{
	Crawler* c = (Crawler*)param;
	c->Crawl();
	return 0;
}

DWORD WINAPI statistic_timer(void* param)
{
	Crawler* c = (Crawler*)param;
	std::ofstream ofs("exprb.csv");
	std::ofstream ofs_2("http_code.csv");
	std::ofstream ofs_3("page_size.csv");
	//std::ofstream ofs("expra.csv");
	//std::ofstream ofs_1("crawl_delay.csv");
	ofs<<"elapse_time,crawl_non_robot,pending_link,total__link,download,resolved_host,valid_robot,non_z_robot,attempt_robot,pending_ip,total_ip,crawl_speed,download_speed\n";
	while(1)
	{
		Sleep(2000);
		if(c->statistic.resolved_host > 1000)
		{			
			
			//printf("Stop Here [%d]\n",c->statistic.none_z_delay_robots.size());
			//for(int i=0;i<c->statistic.none_z_delay_robots.size();i++)
			//ofs_1<<c->statistic.none_z_delay_robots[i]<<"\n";
	  //   	ofs_1.flush();
			for(int i=0;i<c->statistic.HTTP_codes.size();i++)
				ofs_2<<c->statistic.HTTP_codes[i]<<"\n";
	        ofs_2.flush();

			for(int i=0;i<c->statistic.page_sizes.size();i++)
				ofs_3<<c->statistic.page_sizes[i]<<"\n";
	        ofs_3.flush();

			for(int i=0;i<c->thread_num;i++)
			{
				TerminateThread(c->handles[i],0);
			}
			TerminateThread(c->timer,0);

			ExitThread(0);
		}
	
		ofs<<c->statistic.elapse_time<<","<<(double)c->statistic.crawled_non_robot_links/1000<<","<<c->statistic.pending_links/1000<<","<<c->statistic.parsed_out_links/1000<<","<<c->statistic.download_size/1000000<<",";
		ofs<<c->statistic.resolved_host/1000<<","<<c->statistic.valid_robot/1000<<","<<c->statistic.none_z_delay_robots.size()/1000<<","<<c->statistic.total_robot_attempt/1000<<","<<c->statistic.pending_ip/1000<<","<<c->statistic.Total_Ip/1000<<",";
		ofs<<(double)c->statistic.crawled_non_robot_links/(c->statistic.elapse_time)<<","<<(double)c->statistic.download_size/((1000000)*(c->statistic.elapse_time))<<"\n";
		ofs.flush();

		if(c->statistic.crawled_non_robot_links > 1000)
		{
			cout<<"[  "<<c->statistic.elapse_time<<"] C "<<c->statistic.crawled_non_robot_links/1000<<"K/"<<c->statistic.pending_links/1000<<"K/";
			cout<<c->statistic.parsed_out_links/1000<<"K D "<<c->statistic.download_size/1000000<<" H "<<c->statistic.resolved_host/1000<<"K R "<<c->statistic.valid_robot/1000;
			cout<<"K/"<<c->statistic.total_robot_attempt/1000<<"K IP "<<c->statistic.pending_ip/1000<<"K/"<<c->statistic.Total_Ip/1000<<"K S ";
			cout<<"NZ "<<c->statistic.none_z_delay_robots.size()/1000<<" ";
			cout<<(double)c->statistic.crawled_non_robot_links/(c->statistic.elapse_time)<<" ("<<(double)c->statistic.download_size/((1000000)*(c->statistic.elapse_time))<<" Mbps)"<<"\n"; 
		}
		else
		{
			if(c->statistic.elapse_time !=0 )
			{
			cout<<"[  "<<c->statistic.elapse_time<<"] C "<<c->statistic.crawled_non_robot_links<<"/"<<c->statistic.pending_links<<"/";
			cout<<c->statistic.parsed_out_links<<" D "<<c->statistic.download_size/(1000000)<<" H "<<c->statistic.resolved_host<<" R "<<c->statistic.valid_robot;
			cout<<"/"<<c->statistic.total_robot_attempt<<" IP "<<c->statistic.pending_ip<<"/"<<c->statistic.Total_Ip<<" S ";
			cout<<"NZ "<<c->statistic.none_z_delay_robots.size()<<" ";
			cout<<(double)c->statistic.crawled_non_robot_links/(c->statistic.elapse_time)<<" ("<<(double)c->statistic.download_size/((1000000)*(c->statistic.elapse_time))<<" Mbps)"<<"\n"; 
			}
		}		
	}
}

void Crawler::StartThreads(int nThreads) // how many threads to run 
{	
	handles = (HANDLE*)malloc(sizeof(HANDLE*)*nThreads*2);
    for(int i=0;i<nThreads;i++)
	{
	    HANDLE R_handle = CreateThread(NULL,0,crawl,this,0,0);
		handles[2*i] = R_handle;
		HANDLE C_handle = CreateThread(NULL,0,resolve,this,0,0);
		handles[2*i + 1] = C_handle;
		thread_num+=2;
	}

	if(nThreads > 1)
	{
		timer = CreateThread(NULL,0,statistic_timer,this,0,0);
	}

	WaitForMultipleObjects(2*nThreads,handles,true, INFINITE);
	WaitForSingleObject(timer,INFINITE);
}
