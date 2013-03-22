/*
Author: Ke Wang	
UIN: 822002009
*/
#include "stdafx.h"
#include "socket.h"
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

int substr(char* out_string, char* start, char* end)
{
	if(out_string == NULL || start == NULL || end == NULL)
	{
		printf("Null pointer.\n");
		return ERROR_FAIL;
	}
	
	if(end - start <= 0)
	{
		printf("Start should no more than end.\n ");
		return ERROR_FAIL;
	}
	int size = end - start + 1 ;
	memcpy(out_string,start,size);
	out_string[size] = '\0';
	return ERROR_SUCCESS;
}

int right_trim(char* str)
{
	if(!str)
		return 0;
	while(*str != '\0')
	{
		str++;
	}

	while(1)
	{
		if(*str == '\r' || *str == '\n' || *str == ' ')
		{
			*str = '\0';
			str--;
		}
		break;
	}
	return 1;
}

int split(char* in_string, char* out_string[],int size,int* count, char* seperator)
{
	if(in_string == NULL || out_string == NULL || seperator == NULL)
	{
		printf("Null pointer.\n");
		return ERROR_FAIL;
	}
	
	char* pos_end;
	if((pos_end = strstr(in_string,seperator)) == NULL)
	{
		pos_end = in_string + strlen(in_string);
	}

	char* pos_start = in_string;
	*count = 0;
	while(*count < size)
	{
		if(in_string + strlen(in_string) - 1 < pos_start)
		{
			break;
		}

		if(substr(out_string[*count],pos_start,pos_end - 1) == ERROR_FAIL)
		{
			printf("Tmp error.\n");
			return ERROR_FAIL;
		}

		(*count)++;
		pos_start = pos_end + 1;
		if((pos_end = strstr(pos_start,seperator)) == NULL)
		{
			pos_end = in_string + strlen(in_string);
		}
	}

	return ERROR_SUCCESS;
}

int parse_parameter(char* param_string, PARAM* params)
{
	if(param_string == NULL || params == NULL || strlen(param_string) == 0)
		return ERROR_FAIL;

	char* tmp[8];
	for(int i = 0 ;i<8; i++)
	{
		tmp[i] = (char*)malloc(32);
		memset(tmp[i],0,32);
	}

	int count = 0;
	if(split(param_string,tmp,8,&count,"&") == ERROR_FAIL)
	{
		printf("Split error.\n");
		return ERROR_FAIL;
	}

	for(int i = 0 ;i<8; i++)
	{
		if(strlen(tmp[i]) == 0)
			break;
		if(substr(params[i].key,tmp[i],strstr(tmp[i],"=")-1) == ERROR_FAIL)
		{
			printf("Substr error.\n");
			return ERROR_FAIL;
		}
		if(substr(params[i].value,strstr(tmp[i],"=") + 1,tmp[i] + strlen(tmp[i]) - 1) == ERROR_FAIL)
		{
			printf("Substr error.\n");
			return ERROR_FAIL;
		}
	}

	return ERROR_SUCCESS;
}

int parse_url(char* strUrl,char* host_name,char* port_string,char* item_string, char* param_string)
{
	if(strUrl == NULL || host_name == NULL || port_string == NULL || item_string == NULL || param_string == NULL)
	{	
		printf("Null pointer.\n");
		return ERROR_FAIL;
	}
	
	/*get host name or IP address or parameter*/	
	char* pos_colon,*pos_slash,*pos_question;
	if((pos_colon = strstr((char*)strUrl,":")) != NULL)
	{
		if(substr((char*)host_name,(char*)strUrl, pos_colon - 1) == ERROR_FAIL)
		{
			printf("port_string error.\n");
			return ERROR_FAIL;
		}

		if((pos_slash = strstr((char*)strUrl,"/")) != NULL)
		{
			if(substr((char*)port_string,(char*)pos_colon + 1, pos_slash - 1) == ERROR_FAIL)
			{
				printf("port_string error.\n");
				return ERROR_FAIL;
			}
			
			if(((pos_question = strstr((char*)strUrl,"?")) != NULL) || (((pos_question = strstr((char*)strUrl,"#")) != NULL)))
			{
				if(substr((char*)item_string,(char*)pos_slash,pos_question -1) == ERROR_FAIL)
				{
					printf("item_string error.\n");
					return ERROR_FAIL;
				}
				
				if(substr((char*)param_string,(char*)pos_question + 1,(char*)strUrl + strlen((char*)strUrl) -1) == ERROR_FAIL)
				{
					printf("param_string error.\n");
					return ERROR_FAIL;
				}
			}
			else
			{
				if(substr((char*)item_string,(char*)pos_slash,(char*)strUrl + strlen((char*)strUrl) -1) == ERROR_FAIL)
				{
					printf("item_string error.\n");
					return ERROR_FAIL;
				}
			}	
		}
		else
		{
			if(substr((char*)port_string,(char*)pos_colon + 1, (char*)strUrl + strlen((char*)strUrl) -1) == ERROR_FAIL)
			{
				printf("port_string error.\n");
				return ERROR_FAIL;
			}
		}
	}
	else if((pos_slash = strstr((char*)strUrl,"/")) != NULL)
	{
		if(substr((char*)host_name,(char*)strUrl,pos_slash - 1) == ERROR_FAIL)
		{
			printf("host_name error.\n");
			return ERROR_FAIL;
		}
		
		if(((pos_question = strstr((char*)strUrl,"?")) != NULL) || (((pos_question = strstr((char*)strUrl,"#")) != NULL)))
		{
			if(!substr((char*)item_string,(char*)pos_slash,pos_question -1))
			{
				printf("item_string error.\n");
				return ERROR_FAIL;
			}

			if(substr((char*)param_string,(char*)pos_question + 1,(char*)strUrl + strlen((char*)strUrl) -1) == ERROR_FAIL)
			{
				printf("item_string error.\n");
				return ERROR_FAIL;
			}
		}

		if(substr((char*)item_string,(char*)pos_slash,(char*)strUrl + strlen((char*)strUrl) -1) == ERROR_FAIL)
		{
			printf("item_string error.\n");
			return ERROR_FAIL;
		}
	}
	else
	{
		strcpy(host_name,strUrl);
		item_string[0] = '/';
	}
	
	return ERROR_SUCCESS;
}


int is_chunked(char* precv_buf)
{
	if(!precv_buf)
		return 0;
	char* start_pos= strstr(precv_buf,"Transfer-Encoding");
	if(start_pos == NULL)
	{
		return -1;
	}
	start_pos = strstr(start_pos,":");
	char*end = strstr(start_pos,"\r");
	char type[8];

	if(substr(type,start_pos + 2,end - 1) == ERROR_FAIL)
	{
		return 0;
	}
	
	if(strcmp(type,"chunked") == 0)
		return 1;
	return -1;
}

int fetch_size(char* start)
{
	if(!start)
		return -1;
	else if(isalpha(start[0]))
		return -1;
	char hex_string[8];
	sprintf(hex_string,"0X%s",start);
	return strtol(hex_string,NULL,16);
}

char* get_line(char* precv_buf,char* out_buf)
{
	if(!precv_buf || !out_buf)
		printf("NULL pointer.\n");
	
	char* line_start = precv_buf;
	int count = 0;

	while(line_start[count] != '\n')
	{
		out_buf[count] = line_start[count];
		if(line_start[count] == '\0')
		{
			return NULL;
		}
		count++;
	}
	out_buf[count -1] = '\0';
	line_start += count + 1;
	return line_start;
}

int startup()
{
	WORD Version;
	WSADATA Data;
	Version = MAKEWORD(2, 2);
	int ret  = WSAStartup(Version, &Data);
	if(ret != ERROR_SUCCESS)
	{
		printf("Winsock DLL not found.\n");
		return ERROR_FAIL;
	}

	if (LOBYTE(Data.wVersion) != 2 || HIBYTE(Data.wVersion) != 2 )
	{
		/* Tell the user that we could not find a usable WinSock DLL.*/
		printf("Server: The dll do not support the Winsock version %u.%u!\n", LOBYTE(Data.wVersion), HIBYTE(Data.wVersion));
		return ERROR_FAIL;
	}
	
	return ERROR_SUCCESS;
}


int get_address(char* host_name,DWORD* address)
{	
	if(!host_name || !address)
		return ERROR_FAIL;
	
	hostent* host_info;

	if((host_info = gethostbyname(host_name)) == NULL)
	{
		printf("Get host by name with error = %d\n",WSAGetLastError());
		return ERROR_FAIL;
	}
	*address = *(DWORD*)host_info->h_addr;

	if(*address == INADDR_NONE)
	{
		printf("Ipv4 address illegal.\n");
	}

	return ERROR_SUCCESS;
}

bool Socket::sock_create()
{
	if((client = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == INVALID_SOCKET)
	{
		printf("Fail to create socket with error = %d.\n",WSAGetLastError());
		throw;//return ERROR_FAIL;
	}

	bool option = true;
	if(setsockopt(client,SOL_SOCKET,SO_REUSEADDR,(char*)&option,sizeof(bool)) == SOCKET_ERROR)
	{
        printf("Setsockopt for SO_REUSEADDR failed with error: %u\n", WSAGetLastError());
		return ERROR_FAIL;
	}

	LINGER linger;
	linger.l_onoff = 0;
	if(setsockopt(client,SOL_SOCKET,SO_LINGER,(char*)&linger,sizeof(LINGER)) == SOCKET_ERROR)
	{
		printf("Setsockopt for SO_LINGER failed with error: %u\n", WSAGetLastError());
		return ERROR_FAIL;
	}

	return ERROR_SUCCESS;
}

bool Socket::sock_connect(DWORD address,WORD port)
{	
	SOCKADDR_IN server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = address;
	server.sin_port = htons(port);

	if(connect(client,(const sockaddr*)&server,sizeof(sockaddr))
		== SOCKET_ERROR)
	{
		printf("Fail to connect to server with error = %d.\n",WSAGetLastError());
		return ERROR_FAIL;
	}

	return ERROR_SUCCESS;
}

bool Socket::sock_send(char* send_buf,int count)
{
	if(send(client,send_buf,count,0) == SOCKET_ERROR)
	{
		printf("Fail to send to server with error = %d.\n",WSAGetLastError());
		return false;
	}
	return true;
}

bool Socket::sock_recv()
{
	precv_buf = (char*)malloc(recv_buf_size);
	memset(precv_buf,0,recv_buf_size);
	int recv_count = 0;
	fd_set fd;
	FD_ZERO(&fd);
	FD_SET(client, &fd);
	int ret;
	timeval timeout;
	timeout.tv_sec = 30;
	timeout.tv_usec = 0;
	while(1)
	{
		if((ret = select(0, &fd, NULL,NULL, &timeout)) > 0)  
		{
			if(total_recv_count + THRESHOLD < recv_buf_size)
			{	
				recv_count = recv(client,precv_buf + total_recv_count,recv_buf_size - total_recv_count,0);
				if(recv_count == 0)
				{
					break;
				}
				else if(recv_count == SOCKET_ERROR )
				{
					printf("Unexpected occur when recv with error = %d.\n",WSAGetLastError());
					return false;
				}
				else total_recv_count += recv_count;
			}
			else
			{
				char* new_buf = (char*)malloc(2 * recv_buf_size);
				memset(new_buf,0,2 * recv_buf_size);
				memcpy(new_buf,precv_buf,recv_buf_size);
				recv_buf_size *= 2;
				free(precv_buf);
				precv_buf = new_buf;
			}
		}
		else if(ret == 0)
		{
			printf("Recv time out.\n");
			break;
		}
		else
		{
			printf("Unexpected occur when recv with error = %d.\n",WSAGetLastError());
			return false;
		}
		
	}
	precv_buf[total_recv_count] = '\0';
	return true;
}

int Socket::get_header_size()
{
	const char* line_start = get_header();
	if(!line_start)
		return -1;
	std::stringstream ss(line_start);
	std::string line;
	int head_size = 0;

	while(!ss.eof())
	{
		try
		{
			std::getline(ss,line);
		}catch(...)
		{
			printf("Getline exception\n.");
			return -1;
		}
		head_size += (line.size() + 1);
		if(line[0] == '\r')
			break;
	}
	
	return head_size;
}

const char* Socket::get_body()
{
	if(get_header_size() == -1 || get_header() == NULL)
		return NULL;
	const char* ret = get_header() + get_header_size();
	return ret;
}

int Socket::get_body_size()
{
	const char* body = get_body();	
	if(!body)
		return -1;
	int remain_count =0;
	while(*body != '\0')
	{
		body++;
		remain_count++;
	}
	return remain_count;
}

int Socket::get_chunked_body_size()
{
	const char* body = get_body();	
	std::stringstream ss(body);
	bool flag = false;
	int size = 0;
	int line_num = 1;
	while(!ss.eof())
	{
		string line;
		getline(ss,line);
		if(flag || line_num == 1)
		{
			int ret = fetch_size((char*)line.c_str());
			if(ret = -1)
				return 0;
			size += fetch_size((char*)line.c_str());
			flag = false;
		}
		if(line == "\r\n")
			flag = true;
		line_num++;
	}

	return size;
}