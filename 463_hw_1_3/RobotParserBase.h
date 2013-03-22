// RobotParserBase.h
#pragma once

#ifdef _WIN64
#ifdef _DEBUG
#pragma comment (lib, "Robot_Debug_x64.lib")
#else
#pragma comment (lib, "Robot_Release_x64.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment (lib, "Robot_Debug_win32.lib")
#else
#pragma comment (lib, "Robot_Release_win32.lib")
#endif
#endif

class RobotParserBase {
public:
	virtual bool	AddRobotData (char *buf, int len, char *userAgent) = 0;
	virtual bool	VerifyAllowed (char *request) = 0;
	virtual int		GetCrawlDelay (void) = 0;
	virtual int		RecordsFound (void) = 0;
};

extern "C" RobotParserBase* SpawnNewRobotParser (void);