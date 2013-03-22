#pragma once
/*
Author: Ke Wang	
UIN: 822002009
*/

#pragma comment (lib, "ws2_32.lib") 

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>
#include <time.h> 

#define URL_MAX_SIZE 128
#define PORT_MAX_SIZE 4
#define HOST_MAX_SIZE 64
#define ITEM_MAX_SIZE 64
#define PARAM_MAX_SIZE 32
#define PARAM_PAIR_MAX_SIZE 8

//#define ERROR_SUCCESS 0
#define ERROR_FAIL 1

#define SEND_TIME_OUT 1000
#define RECV_TIME_OUT 6000

#define K 1024
#define THRESHOLD (12*K)

#define SEND_BUF_MAX K
#define RECV_BUF_MAX (8*K)

typedef struct param
{
	char key[8];
	char value[8];
}PARAM;


extern int substr(char* out_string, char* start, char* end);
extern int right_trim(char* str);
extern int split(char* in_string, char* out_string[],int size,int* count, char* seperator);
extern int parse_parameter(char* param_string, PARAM* params);
extern int parse_url(char* strUrl,char* host_name,char* port_string,char* item_string, char* param_string);
extern int is_chunked(char* precv_buf);
extern int fetch_size(char* start);
extern char* get_line(char* precv_buf,char* out_buf);
extern int startup();
extern int get_address(char* host_name,DWORD* address);

class Socket
{
private:
	SOCKET client;   /*sock*/
	char  *precv_buf; /*recv buf head*/
	int recv_buf_size;/*buf size*/
	int total_recv_count;/*total recv bytes*/
public:
	Socket():precv_buf(NULL),recv_buf_size(RECV_BUF_MAX),total_recv_count(0){startup();}
	bool sock_create();
	bool sock_connect(DWORD address,WORD port);
	bool sock_send(char* send_buf,int count);
	bool sock_recv();
	bool validate_response();
	bool print_response();
	const char* get_body();
	int get_body_size();
	int get_chunked_body_size();
	const char* get_header(){return precv_buf;};
	int get_header_size();
	int get_full_size(){return total_recv_count;}
	void reset(){if(precv_buf){free(precv_buf);precv_buf = NULL;}recv_buf_size = RECV_BUF_MAX;total_recv_count = 0;}
	bool close(){closesocket(client);return 1;}
	~Socket(){free(precv_buf);closesocket(client);}
};
