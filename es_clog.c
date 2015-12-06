/*
* 有效性检查
* 多线程
* 系统文件太大自动清理

* 日志缓存 换成全局变量 是否更好
*/

/*
* 10M ~= 300 000 line hello work
*/
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
//#include <sys/star.h>
#include <sys/stat.h>
#include <unistd.h>
//#include <syslog.h> /* support for syslog */

/* in direct for function arg */
#define IN

/* out direct for function arg */
#define OUT

/* max length of file name len */
#define ES_LOG_FILE_NAME_LEN	64

/* max length of system cmd len */
#define ES_LOG_SYSTEM_CMD_LEN 	256

/* max length of log time buffer len */
#define ES_LOG_TIME_LEN 	64

/* max length of log buffer */
#define ES_LOG_BUFFER_LEN	1024

typedef struct tagESLogHandle
{
	/* log file name */
	char szFileName[ES_LOG_FILE_NAME_LEN];

	/* log file max size */
	unsigned long ulLogFileMaxSize;
	
	/* log handle */
	FILE *hFileHandle;
}ES_LOG_HANDLE_S;

//#define ES_LOG(pstLogHandle, ) ES_writeLog(IN ES_LOG_HANDLE_S *pstLogHandle, IN const char *pcFormat, IN ...)
	
/* global log handle */
ES_LOG_HANDLE_S gLogHandle = {
	.szFileName = {0},
	.ulLogFileMaxSize = 0,
	.hFileHandle = NULL
};
	
void ES_tryOpenFile(IN ES_LOG_HANDLE_S *pstLogHandle)
{
	/* may be failed to open file */
	pstLogHandle->hFileHandle = fopen(pstLogHandle->szFileName, "a+");
	if (0 == pstLogHandle->hFileHandle)
	{
		printf("Failed to open file(%s).\r\n", pstLogHandle->szFileName);
		exit(-1);
	}
	return;
}
/* log function */
void ES_writeLog(IN ES_LOG_HANDLE_S *pstLogHandle, IN const char *pcFormat, IN ...)
{
	int iRet = 0;
	//int  iLogBufferLen = 0;						/* log buffer count */
	//char szLogBuffer[ES_LOG_BUFFER_LEN] = {0};	/* log buffer */
	int  iTImeBufferLen = 0;					/* log time buffer count */
	char szTimeBuffer[ES_LOG_TIME_LEN] = {0};	/* log time buffer */
	char szSystemCmd[ES_LOG_SYSTEM_CMD_LEN] = {0};						/* system call cmd */
	

	/* get current time */
	time_t now;
	time(&now);
	struct tm *local;
	local = localtime(&now);

	/* get current time buffer */
	iTImeBufferLen = snprintf(szTimeBuffer, ES_LOG_TIME_LEN, "[%04d-%02d-%02d %02d:%02d:%02d]", 
			local->tm_year + 1900, local->tm_mon,
			local->tm_mday, local->tm_hour, 
			local->tm_min, local->tm_sec);
	
	/* file oprator */
	
	//check file state
	struct stat stStat = {0};
	iRet = stat(pstLogHandle->szFileName, &stStat);
	if (0 != iRet)
	{
		printf("Failed to get stat for file(%s) may be not exist.\r\n", pstLogHandle->szFileName);
	}
	else /* success to  get file stat */
	{
		if (pstLogHandle->ulLogFileMaxSize < stStat.st_size)
		{
			/* move log to backup */
			(void) snprintf(szSystemCmd, ES_LOG_SYSTEM_CMD_LEN,
							"tar -czf %s_%04d%02d%02d_%02d%02d%02d.tar.gz %s;rm -rf %s;touch %s;",
							pstLogHandle->szFileName, 
							local->tm_year + 1900, local->tm_mon,
							local->tm_mday, local->tm_hour, 
							local->tm_min, local->tm_sec,
							pstLogHandle->szFileName,
							pstLogHandle->szFileName,
							pstLogHandle->szFileName);
			iRet = system(szSystemCmd);
			if (0 != iRet)
			{
				printf("Failed to call system:%s.\r\n", szSystemCmd);
				/* 文件占用是不能删除 例如 tail */
			}
		}
	}
	
	ES_tryOpenFile(pstLogHandle);
	
	//FILE *hFileHandle = fopen(pstLogHandle->szFileName, "a+");
	fwrite(szTimeBuffer, sizeof(char)/* each size bytes long */, iTImeBufferLen, pstLogHandle->hFileHandle);
	//fwrite(szLogBuffer, sizeof(char)/* each size bytes long */, strlen(szLogBuffer), pstLogHandle->hFileHandle);
	
	/* get arg */
	va_list args;
	va_start(args, pcFormat);
	vfprintf(pstLogHandle->hFileHandle, pcFormat, args);
	va_end(args);
	
	fclose(pstLogHandle->hFileHandle);

	//syslog(LOG_INFO,wzLog); /* support for syslog */
	return ;
}

int main()
{
	strncpy(gLogHandle.szFileName, "mylog.txt", ES_LOG_FILE_NAME_LEN);
	gLogHandle.ulLogFileMaxSize = 10 * 1024 *1024;
	
	while(1)
		ES_writeLog(&gLogHandle, "Hello %s\r\n.", "World");
	return 0;
}
