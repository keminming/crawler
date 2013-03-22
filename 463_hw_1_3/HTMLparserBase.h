// HTMLparserBase.h
#pragma once
#include <string>
#include <queue>
using namespace std;

#ifdef _WIN64
#ifdef _DEBUG
#pragma comment (lib, "HTMLparserLib_Debug_x64.lib")
#else
#pragma comment (lib, "HTMLparserLib_Release_x64.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment (lib, "HTMLparserLib_Debug_win32.lib")
#else
#pragma comment (lib, "HTMLparserLib_Release_win32.lib")
#endif
#endif

#define MAX_VALID_URL			2048	// max URL size

class ParsedLink {
public:
		string			host;			// hostname or IP
		string			request;		// request portion
		unsigned short	port;			// port number
};

class HTMLparserBase {
public:
	// input: parentURL must be a NULL-terminated, absolute URL starting with http://
	// return: pointer to a queue of links; NULL if parentURL is invalid 
	virtual queue<ParsedLink> *Parse (char *buf, int len, char *parentURL) = 0;
};

extern "C" HTMLparserBase* SpawnNewHTMLParser (void);
