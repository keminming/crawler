#include "stdafx.h"
#include "http.h"
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <string.h>
#include <memory>


const int M = 1000000; 

int HTTP::download(ParsedLink link)
{
	s.reset();
	if(s.sock_create() == ERROR_FAIL)
		return ERROR_FAIL;
	DWORD address;
	if(get_address((char*)link.host.c_str(),&address) == ERROR_FAIL)
		return ERROR_FAIL;
	if(s.sock_connect(address,link.port) == ERROR_FAIL)
		return ERROR_FAIL;
	

	std::unique_ptr<char> send_buf(new char[2048]);
	int count = 0;
	count += sprintf(send_buf.get() + count,"GET %s HTTP/1.1\r\n",link.request.c_str());
	count += sprintf(send_buf.get() + count,"Host: %s\r\n",link.host.c_str());
	count += sprintf(send_buf.get() + count,"User-agent: Crawler-ke-wang-1.2\r\n");
	count += sprintf(send_buf.get() + count,"Connection: close\r\n");
	count += sprintf(send_buf.get() + count,"\r\n");
	send_buf.get()[count] = '\0';

	if(!s.sock_send(send_buf.get(),count))
		return ERROR_FAIL;

	if(!s.sock_recv())
		return ERROR_FAIL;
	try
	{
	if((head_start = s.get_header()) == NULL)
		return ERROR_FAIL;
	
	if((head_size = s.get_header_size()) == -1)
		return ERROR_FAIL;

	if((body_start = s.get_body()) == NULL)
		return ERROR_FAIL;

	if((body_size = s.get_body_size()) == -1)
		return ERROR_FAIL;
	}
	catch(...)
	{
		return ERROR_FAIL;
	}
	full_size = s.get_full_size();
	local_stat.download_size += full_size;

	if(!s.close())
		return ERROR_FAIL;

	return ERROR_SUCCESS;
}

int HTTP::parse_header()
{
	if(head_start[0] != 'S' && head_start[0] != 'H')
		return ERROR_FAIL;
	stringstream s(head_start);
	string status_line;
	getline(s,status_line);
	vector<string> tokens;
	string token;
	stringstream sl(status_line);
	while(!sl.eof())
	{
		getline(sl,token,' ');
		tokens.push_back(token);
	}

	this->status = atoi(tokens[1].c_str());
	return ERROR_SUCCESS;
}

string HTTP::get_redirect_url()
{
	string ret = get_head_start();
	try
	{
		ret = ret.substr(ret.find("Location"));
		ret = ret.substr(ret.find(":") + 2,ret.find("\r") - 1 - (ret.find(":") + 2) + 1);
	}
	catch(...)
	{
		throw;
	}
	return ret;
}


