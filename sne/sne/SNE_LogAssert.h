#ifndef _PRIORITY_LOG_ASSERT_
#define _PRIORITY_LOG_ASSERT_

typedef void (*LogCB)(const char*);

/**
 * 以下宏方便调用，调用前请先调用 UTL_LogAssert::Init(...);
 */
#define UTL_LOG_NORMAL		SNE_LogAssert::PreSetting(__FUNCTION__, __LINE__);SNE_LogAssert::LogNormal
#define UTL_LOG_HIGH		SNE_LogAssert::PreSetting(__FUNCTION__, __LINE__);SNE_LogAssert::LogHigh
#define UTL_ASSERT			SNE_LogAssert::PreSetting(__FUNCTION__, __LINE__);SNE_LogAssert::Assert


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class SNE_LogAssert
{
public:
	/**
	 * log使用前需先init
	 *
	 * @param is_final 是否是最终版，如果是则只有UTL_LogHigh能打印信息，UTL_LogNormal的打印被取消
	 * @param path log文件的保存路径
	 * @param filemaxsize 每个log文件最大尺寸，当大于最大值时从新生成一个log文件
	 * @param out_file 是否将log保存成文件
	 * @param out_vc 是否在vc的输出窗口打印信息
	 * @param out_cmdline 是否向控制台输出
	 */
	static void Init(bool is_final, const char* path = "logs", int filemaxsize = 1024*1024*16, bool enable_memory_info = false, bool out_file = true, bool out_vc = true, bool out_cmdline = true);

	/**
	 * 使用后释放
	 */
	static void Exit();

	/**
	* 注册外部log
	*/
	static void RegisterLogCB(LogCB lognormal, LogCB loghigh);

	/**
	 * 用于打印一般信息。当宏 UTL_LogAssertInit的参数 is_final = true 时，此函数为空函数
	 *
	 * @param type log的用户自定义类别，可方便用于log分析时筛选信息
	 */
	static void LogNormal(const char* type, const char* format, ...);

	/**
	 * 用于打印重要信息，最终发行版会保留此信息
	 */
	static void LogHigh(const char* type, const char* format, ...);

	/**
	 * 当 UTL_LogAssertInit的参数 is_final = true 时，如果assert失败只会打印log, 不会使程序中断
	 * 否则，assert失败会使程序中断并弹出对话框
	 */
	static void Assert(bool is_true, const char* format = "", ...);


	/**
	 * 调用前设置调用函数名，行数
	 */
	static void PreSetting(const char* func_name, int line);
};

#endif 

 
