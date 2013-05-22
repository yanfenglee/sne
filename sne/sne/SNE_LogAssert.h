#ifndef _PRIORITY_LOG_ASSERT_
#define _PRIORITY_LOG_ASSERT_

typedef void (*LogCB)(const char*);

/**
 * ���º귽����ã�����ǰ���ȵ��� UTL_LogAssert::Init(...);
 */
#define UTL_LOG_NORMAL		SNE_LogAssert::PreSetting(__FUNCTION__, __LINE__);SNE_LogAssert::LogNormal
#define UTL_LOG_HIGH		SNE_LogAssert::PreSetting(__FUNCTION__, __LINE__);SNE_LogAssert::LogHigh
#define UTL_ASSERT			SNE_LogAssert::PreSetting(__FUNCTION__, __LINE__);SNE_LogAssert::Assert


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class SNE_LogAssert
{
public:
	/**
	 * logʹ��ǰ����init
	 *
	 * @param is_final �Ƿ������հ棬�������ֻ��UTL_LogHigh�ܴ�ӡ��Ϣ��UTL_LogNormal�Ĵ�ӡ��ȡ��
	 * @param path log�ļ��ı���·��
	 * @param filemaxsize ÿ��log�ļ����ߴ磬���������ֵʱ��������һ��log�ļ�
	 * @param out_file �Ƿ�log������ļ�
	 * @param out_vc �Ƿ���vc��������ڴ�ӡ��Ϣ
	 * @param out_cmdline �Ƿ������̨���
	 */
	static void Init(bool is_final, const char* path = "logs", int filemaxsize = 1024*1024*16, bool enable_memory_info = false, bool out_file = true, bool out_vc = true, bool out_cmdline = true);

	/**
	 * ʹ�ú��ͷ�
	 */
	static void Exit();

	/**
	* ע���ⲿlog
	*/
	static void RegisterLogCB(LogCB lognormal, LogCB loghigh);

	/**
	 * ���ڴ�ӡһ����Ϣ������ UTL_LogAssertInit�Ĳ��� is_final = true ʱ���˺���Ϊ�պ���
	 *
	 * @param type log���û��Զ�����𣬿ɷ�������log����ʱɸѡ��Ϣ
	 */
	static void LogNormal(const char* type, const char* format, ...);

	/**
	 * ���ڴ�ӡ��Ҫ��Ϣ�����շ��а�ᱣ������Ϣ
	 */
	static void LogHigh(const char* type, const char* format, ...);

	/**
	 * �� UTL_LogAssertInit�Ĳ��� is_final = true ʱ�����assertʧ��ֻ���ӡlog, ����ʹ�����ж�
	 * ����assertʧ�ܻ�ʹ�����жϲ������Ի���
	 */
	static void Assert(bool is_true, const char* format = "", ...);


	/**
	 * ����ǰ���õ��ú�����������
	 */
	static void PreSetting(const char* func_name, int line);
};

#endif 

 
