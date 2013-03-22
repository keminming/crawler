#pragma once
#include "HTMLparserBase.h"
#include "socket.h"


class Statistic
{
public:
	int total_links;
	int links;
	int unique_links;
	double download_size;
	int crawled_links;
	Statistic():total_links(0),links(0),unique_links(0),download_size(0),crawled_links(0){}
};

class HTTP
{
private:
	Socket s;
	int status;
	const char* head_start;
	int head_size;
	const char* body_start;
	int body_size;
	int full_size;

public:
	Statistic local_stat;
	HTTP():status(0),head_start(NULL),head_size(0),body_start(NULL),body_size(0),full_size(0){}
	int download(ParsedLink pl);
	int parse_header();
	int get_status(){return status;}
	unsigned int get_full_size(){return full_size;}
	const char* get_body_start(){return body_start;}
	int get_body_size(){return body_size;}
	const char* get_head_start(){return head_start;}
	int get_head_size(){return head_size;}
	bool parse_links(char *buf, int len, char *parentURL);
	string get_redirect_url();
	~HTTP(){};
};

