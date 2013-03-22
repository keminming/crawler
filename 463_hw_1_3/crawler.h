/*
	Author: Ke Wang
	UIN: 822002009
*/

#include <deque>
#include <unordered_map>
#include <string>
#include <queue>
#include <unordered_set>
#include "http.h"
#include "HTMLparserBase.h"
#include "RobotParserBase.h"
#include <windows.h>
#include <memory>

using namespace std;

enum DNS_state
{
	 DNS_PENDING, 
	 BLACKLISTED, 
	 DNS_EXISTS, 
	 DNS_FAIL 
};

enum IP_state
{
	 PENDING, 
	 IDLE, 
	 CRAWLED
};

class Url
{
public:
	int redirectCount;
	ParsedLink pl;
};


class HostRecord 
{
public:
	long IP;
	int status;
	string hostname;
	deque<Url> pendingLinks;
	unordered_map<unsigned short,RobotParserBase*> rpMap;
};

class IPRecord
{
public:
	long ip;
	long timestamp;
	int status;
	deque<shared_ptr<HostRecord>> pendingHosts;
};

class CompareIPs
{
public:
	bool operator()(const shared_ptr<IPRecord>& x,const shared_ptr<IPRecord>& y)
	{
		return x->timestamp > y->timestamp;
	}
};

class CrawlerObject
{  
public:
	Url    url;  // url to download  
	shared_ptr<HostRecord>  h;  // corresponding host  
	shared_ptr<IPRecord>  ipr;  // corresponding IP 

	CrawlerObject(){ipr = NULL; h= NULL;}
}; 


class CrawlerCache
{
public:
	CRITICAL_SECTION q_lock;
	CRITICAL_SECTION hostmap_lock;
	CRITICAL_SECTION ipmap_lock;
	CRITICAL_SECTION heap_lock;
	HANDLE sema;
	queue<shared_ptr<HostRecord>> QpendingDNS;
	unordered_map<string,shared_ptr<HostRecord>> hostMap;
	unordered_map<DWORD,shared_ptr<IPRecord>> ipMap;
	unsigned long pending_links;

	priority_queue<shared_ptr<IPRecord>, vector<shared_ptr<IPRecord>>,CompareIPs> heap;

public:
	CrawlerCache(){pending_links = 0;InitializeCriticalSection(&q_lock); InitializeCriticalSection(&hostmap_lock);InitializeCriticalSection(&ipmap_lock);InitializeCriticalSection(&heap_lock);sema = CreateSemaphore(NULL,0,200,NULL);}
	void InsertNewURL(Url* link);
	shared_ptr<HostRecord>  RemoveHostPendingDNS();
	void AddHostToIP(DWORD IP, shared_ptr<HostRecord>& h);
	void RemoveHeap(CrawlerObject *co);
	void ReinsertHeap(shared_ptr<IPRecord>& ipr);

	~CrawlerCache(){DeleteCriticalSection(&q_lock); DeleteCriticalSection(&hostmap_lock);DeleteCriticalSection(&ipmap_lock);DeleteCriticalSection(&heap_lock);CloseHandle(sema);}
};

class Stat
{
public:
	unsigned long elapse_time;
	unsigned long crawled_non_robot_links;
	unsigned long pending_links;
	unsigned long parsed_out_links;
	unsigned long resolved_host;
	unsigned long valid_robot;
	vector<int> none_z_delay_robots;
	unsigned long total_robot_attempt;
	unsigned long pending_ip;
	unsigned long Total_Ip;
	unsigned long download_size;
	vector<int> HTTP_codes;
	vector<int> page_sizes;
	Stat():elapse_time(0),crawled_non_robot_links(0),pending_links(0),parsed_out_links(0),resolved_host(0),valid_robot(0),total_robot_attempt(0),Total_Ip(0){}
};


class Crawler 
{ 
private:  
	CrawlerCache cc;  // shared among threads  ...    // additional variables
	CRITICAL_SECTION set_lock;
    unordered_set<string> set; 
	string seed;

public:
	int page_num;
	HANDLE* handles;
	HANDLE timer;
	int thread_num;
	CRITICAL_SECTION static_lock;
	Stat statistic;
	Crawler(string seed, int page_num){thread_num = 0;this->seed = seed; this->page_num = page_num;InitializeCriticalSection(&static_lock);InitializeCriticalSection(&set_lock);}
	void StartThreads(int nThreads);  // how many threads to run 
	DWORD Crawl();
	DWORD Resolve();
	bool parse_links(HTMLparserBase* hp, char *buf, int len, char *parentURL);
	bool is_dup(string redirect_ulr);
	bool is_secure(string redirect_ulr);
	void parse_url(string url,ParsedLink* pl);
}; 


