
#define _WIN32_WINNT 0x0501

#include "SNE_LogAssert.h"
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <stdlib.h>
#include <io.h>
#include <direct.h>
#include <assert.h>
#include <time.h>
#include <sys/timeb.h>

//#include "util.h"

#include "Psapi.h" 
#pragma comment(lib, "Psapi.lib")
#pragma warning(disable: 4996)

#define FUNCNAME_LENGTH (128)
#define FILENAME_LENGTH (256)

enum LOG_OUTPUT_CHANNEL
{
	LOG_OUT_FILE = 0x01,
	LOG_OUT_SCREEN = 0x02,
	LOG_OUT_VCOUTPUT = 0x04,
	LOG_OUT_CUSTMIZE = 0x08,		//	由外部Config文件控制
	LOG_OUT_CMDLINE = 0x10,		//	由命令行窗口输出
};

static LogCB sm_lognormal = 0;
static LogCB sm_loghigh = 0;

static int sm_nConfigChannel = 0;
static bool sm_isFinal = false;
static bool sm_enable_memory_log = false;

static char sm_funcname[FUNCNAME_LENGTH];
static unsigned int sm_line = 0;

static int sm_MaxFileSize = 0;

static char sm_csvfile[FILENAME_LENGTH];    // 可分析log
static char sm_log_path[FILENAME_LENGTH];

static int sm_lastMemory = 0;



static int DEF_MAX_LOGFILE_SIZE = (16*1024*1024);
static unsigned int file_count = 0;

static bool g_inited = false;

static bool UseExternalLog()
{
	return (sm_lognormal || sm_loghigh);
}

int CreatDir(char *pDir)
{
	int i = 0;
	int iRet;
	int iLen;
	char* pszDir;

	if(NULL == pDir)
	{
		return 0;
	}

	pszDir = strdup(pDir);
	iLen = strlen(pszDir);

	// 创建中间目录
	for (i = 0;i < iLen;i ++)
	{
		if (pszDir[i] == '\\' || pszDir[i] == '/')
		{ 
			pszDir[i] = '\0';

			//如果不存在,创建
			iRet = _access(pszDir,0);
			if (iRet != 0)
			{
				iRet = _mkdir(pszDir);
				if (iRet != 0)
				{
					return -1;
				} 
			}
			//支持linux,将所有\换成/
			pszDir[i] = '/';
		} 
	}

	iRet = _mkdir(pszDir);
	free(pszDir);
	return iRet;
}

static void Initialize()
{
	if (g_inited)
	{
		UTL_LOG_HIGH("log", "log already inited!!!");
		return;
	}

	sm_MaxFileSize = 0;

	SYSTEMTIME st;
	GetLocalTime(&st);

	char csvfile[64];

	sprintf(csvfile, "%d_%02d_%02d_%02d_%02d_%02d__%04d", GetCurrentProcessId(), st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, file_count++);

	sm_csvfile[0] = 0;


	CreatDir(sm_log_path);
	

	_snprintf(sm_csvfile,FILENAME_LENGTH-1, "%s/%s.csv", sm_log_path, csvfile);
	if( sm_nConfigChannel & LOG_OUT_FILE )
	{
		FILE* fp = fopen(sm_csvfile, "w");
		char* str = "Message, Function, Line, Message Type, Time\n";
		if (fp != 0)
		{
			fseek(fp, 0, SEEK_END);
			fwrite(str, strlen(str), 1, fp);
			fclose(fp);

			//printf("new file open++++++++++++++++++++++++ %s ++++++++++++++++++++++++\n", sm_csvfile);
		}
	}

	g_inited = true;
}

static void Terminate()
{
	sm_csvfile[0] = 0;
	g_inited = false;
}

static void AutoInit()
{
	if (!g_inited)
	{
		SNE_LogAssert::Init(false, "netlog");
	}
}

static void GetMenInfo(int &m,int &p)
{
	static HANDLE h_proc=GetCurrentProcess();

	m=p=0;

	PROCESS_MEMORY_COUNTERS	pmc;
	if(GetProcessMemoryInfo(h_proc,&pmc,sizeof(PROCESS_MEMORY_COUNTERS)))
	{
		m = pmc.WorkingSetSize;
		p = pmc.PeakWorkingSetSize;
	}
}

static void GetLogStr(const char* module1, char* msgBuff, int msgBuffLen, char* buf)
{

	for (int i = 0; i < msgBuffLen; i++)
	{
		// 把输出的逗号改为分号
		if (msgBuff[i] == ',')
		{
			msgBuff[i] = ';';
		}
		// 把换行改成tab
		else if (msgBuff[i] == '\n')
		{
			msgBuff[i] = '\t';
		}
		else if (msgBuff[i] == 0)
		{
			break;
		}
	}

	//add time stamp by liyanfeng
	struct _timeb timebuffer;
	_ftime64_s( &timebuffer );
	char temp[256], timeline[26];
	ctime_s( timeline, 26, &(timebuffer.time) );
	timeline[24] = '\0';
	sprintf(temp, "%.19s.%hu %s", timeline, timebuffer.millitm,	&timeline[20]);

	if (sm_enable_memory_log)
	{
		int memory,memory_peak;
		GetMenInfo(memory,memory_peak);

		DWORD hc=0;
		GetProcessHandleCount(GetCurrentProcess(),&hc);

		_snprintf(buf,1024+512, "[%d/%d(%d) h=%d] - %s, %s, %d, %s, %s\n",memory, memory_peak,  memory-sm_lastMemory, hc, msgBuff, sm_funcname, sm_line, module1, temp);
		sm_lastMemory=memory;
	}
	else
	{
		_snprintf(buf,1024+512, "%s, %s, %d, %s, %s\n", msgBuff, sm_funcname, sm_line, module1, temp);
	}

}

static void printInternal(int channel, const char* module1, char* msgBuff, int msgBuffLen, LogCB external_log = 0)
{
	char buf[1024+512];

	GetLogStr(module1, msgBuff, msgBuffLen, buf);

	if (external_log)
	{
		external_log(buf);
	}

	if (UseExternalLog())
	{
		return;
	}

	if ( channel & LOG_OUT_FILE 
		&& 	sm_csvfile[0] != 0)
	{
		FILE* fp = fopen(sm_csvfile, "a");
		if (fp != 0)
		{
			fseek(fp, 0, SEEK_END);
			fwrite(buf, strlen(buf), 1, fp);
			fclose(fp);
		}

		sm_MaxFileSize += (int)strlen(buf);
		if(sm_MaxFileSize > DEF_MAX_LOGFILE_SIZE)
		{
			Terminate();
			Initialize();
		}
	}

	if ( channel & LOG_OUT_VCOUTPUT )
	{
		OutputDebugStringA(buf);
	}

	if (channel & LOG_OUT_CMDLINE)
	{
		printf("[%s]  %s\n", module1,msgBuff);
	}
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SNE_LogAssert::Init(bool is_final, const char* path /*= "logs"*/, int filemaxsize /*= 1024*1024*16*/, bool enable_memory_info  /*= false*/, bool out_file /*= true*/, bool out_vc /*= true*/, bool out_cmdline /*= true*/ )
{
	sm_isFinal = is_final;
	sm_nConfigChannel = 0;
	sm_enable_memory_log = enable_memory_info;

	if (path != NULL && *path)
	{
		strncpy(sm_log_path, path, FILENAME_LENGTH);
	}
	else 
	{
		strncpy(sm_log_path, "logs", FILENAME_LENGTH);
	}
	
	file_count = 0;

	if (out_file)
	{
		sm_nConfigChannel |= LOG_OUT_FILE;
	}

	if (out_vc)
	{
		sm_nConfigChannel |= LOG_OUT_VCOUTPUT;
	}

	if (out_cmdline)
	{
		sm_nConfigChannel |= LOG_OUT_CMDLINE;
	}


	DEF_MAX_LOGFILE_SIZE = filemaxsize;

	Initialize();

}

void SNE_LogAssert::Exit()
{
	if (!g_inited)
	{
		UTL_LOG_HIGH("log", "log have not inited!!!");
		return;
	}

	Terminate();

}

void SNE_LogAssert::LogNormal( const char* type, const char* format, ... )
{
	if (!UseExternalLog())
	{
		AutoInit();
	}


	if (!sm_isFinal)
	{
		char msg[1024];
		va_list arglist;
		va_start(arglist, format);

		_vsnprintf(msg,1024, format, arglist);


		printInternal(sm_nConfigChannel, type, msg, sizeof(msg), sm_lognormal);
	}
}

void SNE_LogAssert::LogHigh( const char* type, const char* format, ... )
{
	if (!UseExternalLog())
	{
		AutoInit();
	}

	char msg[1024];
	va_list arglist;
	va_start(arglist, format);

	_vsnprintf(msg,1024, format, arglist);

	printInternal(sm_nConfigChannel, type, msg, sizeof(msg), sm_loghigh);
}

void SNE_LogAssert::Assert( bool is_true, const char* format, ... )
{
	if (!UseExternalLog())
	{
		AutoInit();
	}

	if (!is_true)
	{
		char msg[1024];
		va_list arglist;
		va_start(arglist, format);

		_vsnprintf(msg,1024, format, arglist);

		printInternal(sm_nConfigChannel, "ASSERT", msg, sizeof(msg), sm_loghigh);

		if (!sm_isFinal && !UseExternalLog())
		{
			::MessageBoxA(NULL, msg, "assert failed", MB_ICONERROR);
			__asm int 3;
		}
	}
}

void SNE_LogAssert::PreSetting( const char* func_name, int line )
{
	strncpy(sm_funcname, func_name, FUNCNAME_LENGTH);
	sm_line = line;
}

void SNE_LogAssert::RegisterLogCB( LogCB lognormal, LogCB loghigh)
{
	sm_lognormal = lognormal;
	sm_loghigh = loghigh;
}

#pragma warning(default:4996)