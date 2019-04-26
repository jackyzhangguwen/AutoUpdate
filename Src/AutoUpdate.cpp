//
// AutoUpdate.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "AutoUpdate.h"
#include "InternetGetFile.h"
#include "UISkinManager.h"
#include "RMessageBox.h"
#include "AutoUpdateDlg.h"
#include "UpdateThread.h"
#include "tlhelp32.h"

#include "HttpClient.h"
#include "JsonParser.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// 全局字符串常量表数组（数组初始化在InitStringTable()函数中进行）
struct StringStru g_String[STRING_BOTTOM];

// 全局语言代码（不可随意修改其值，否则会导致程序异常！）
enum enLANGUAGE g_LanguageID;

/////////////////////////////////////////////////////////////////////////////
// CAutoUpdateApp

BEGIN_MESSAGE_MAP(CAutoUpdateApp, CWinApp)
	//{{AFX_MSG_MAP(CAutoUpdateApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAutoUpdateApp construction

CAutoUpdateApp::CAutoUpdateApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_bSilenceMode = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CAutoUpdateApp object

CAutoUpdateApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CAutoUpdateApp initialization

BOOL CAutoUpdateApp::InitInstance()
{
	LOG(0, 0, "============ 自动更新程序启动 ============");

	// ================	测试开始 ================

	// CHttpClient httpclient;
	// string strResponse;
	// httpclient.Post("http://httpbin.org/post", "name=akagi201&project=curl", strResponse);
	// CParseJson jsonParser;
	// bool result = jsonParser.ParseJson(strResponse);

	// string value;
	// result = jsonParser.GetString("data", value);
	// ::MessageBox(NULL, value.c_str(), "提示", 0);

	// 嵌套解析
	// Value jsonValue;
	// result = jsonParser.GetObjectValue("form", jsonValue);
	// ::MessageBox(NULL, 
	//	("root.form.name值为：" + jsonValue["name"].asString()).c_str(),
	//	"信息", 
	//	0);

	// ================	测试结束 ================


	// 初始化自绘界面
	CUISkinManager::Init();

	// 初始化全局字符串常量表数组
	InitStringTable();

	// 处理命令行
	if (!ParseCommandLine()) {
		// 清理自绘界面
		CUISkinManager::Uninit();
		return FALSE;
	}

	// 检查程序是否已经在运行，如是则直接退出
	if (IsAppRunning())	{ return FALSE;	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	// Call this when using MFC in a shared DLL
	Enable3dControls();
#else
	// Call this when linking to MFC statically
	Enable3dControlsStatic();
#endif

	// 显示主窗口
	CAutoUpdateDlg dlg;
	m_pMainWnd = &dlg;
	dlg.m_strAppName = m_strAppName;
	dlg.m_bSilenceMode = m_bSilenceMode;
	if (m_bSilenceMode)	{
		// 静默方式执行升级
		m_pMainWnd = NULL;
		dlg.SetConfigFile(GetAppDirectory() + UPDATE_CONFIG_FILENAME);
		dlg.DoUpdate();

		// 等待升级线程完成
		HANDLE hThread = (dlg.m_pUpdateThread)->m_hThread;
		WaitForSingleObject(hThread, INFINITE);
		DWORD dwExit;
		BOOL bRet = GetExitCodeThread(hThread, &dwExit);
       
		if (dwExit != 1) {
			LOG(0, 0, "升级线程出现错误,开始还原UpdateConfig.ini文件.");
			RestoreUpdateCfgIniFile();
		}
	} else {
		int nResponse = dlg.DoModal();
		if (nResponse == IDOK) {
			// TODO: Place code here to handle when the dialog is
			//  dismissed with OK
		} else if (nResponse == IDCANCEL) {
			// && dlg.m_btnCancel.GetWindowTextA() == STRING(STR_BUTTON_SUCCESS_UPGRADE, "完成升级")
			if (dlg.m_strPrompt == STRING(STR_PROMPT_UPGRADE_FINISHED, "升级完毕！")) {
				LOG(0, 0, "升级完成.");
			} else {
				LOG(0, 0, "用户取消升级,开始还原UpdateConfig.ini文件.");
				RestoreUpdateCfgIniFile();
			}
		}
	}

	// 清理自绘界面
	CUISkinManager::Uninit();

	LOG(0, 0, "AutoUpdate程序即将退出!");
	LOG(0, 0, "------------ 自动更新程序结束 ------------");

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

//
// 处理命令行
// 返回值
// TRUE 表示要继续运行后面的程序，
// FALSE 表示终止程序运行
//
BOOL CAutoUpdateApp::ParseCommandLine()
{
	m_strAppName = "MyBrowser.exe";

	CString sConfigFilename = GetAppDirectory() + UPDATE_CONFIG_FILENAME;

	// 从升级配置文件取得最新软件版本号
	try {
		CString sKey = "Version";
		const int BUFFER_SIZE = 512;
		char acBuffer[BUFFER_SIZE] = { 0 };
		GetPrivateProfileString(SECTION_UPDATE,
			sKey.GetBuffer(0),
			"",
			acBuffer,
			BUFFER_SIZE,
			sConfigFilename.GetBuffer(0));
		m_strVersion = (char*)acBuffer;
	} catch (...) {
		m_strVersion = "3.3.0";
	}

	try	{
		CString sKey = "BaseUrl";
		const int BUFFER_SIZE = 512;
		char acBuffer[BUFFER_SIZE] = { 0 };
		GetPrivateProfileString(SECTION_UPDATE,
			sKey.GetBuffer(0),
			"",
			acBuffer,
			BUFFER_SIZE,
			sConfigFilename.GetBuffer(0));
		m_strBaseURL = (char*)acBuffer;
	} catch (...) {
		m_strBaseURL = "http://127.0.0.1:8080/Autoupdate";
	}

	m_strURL = m_strBaseURL + "/" + UPDATE_CONFIG_FILENAME;

	// m_strNotifyWindowTitle = "小房子浏览器自动升级程序";
	m_bSilenceMode = FALSE;

	m_strAppName.TrimLeft();
	m_strAppName.TrimRight();
	if (m_strAppName.IsEmpty()) {
		LOG(0, 0, "无效的应用名称.");
		return FALSE;
	}

	m_strVersion.TrimLeft();
	m_strVersion.TrimRight();
	if (m_strVersion.IsEmpty()) {
		LOG(0, 0, "无效的本地当前版本号，请检查当前目录下UpdateConfig.ini文件配置.");
		return FALSE;
	}

	// 检查升级配置文件，判断是否有新版本的软件可用
	if (CheckUpdate()) {
		if (!m_bSilenceMode) {
			CRMessageBox MsgBox;
			// MsgBox.m_sTitle = m_strAppName + " " + STRING(STR_AUTO_UPDATE, "Auto Update");
			MsgBox.m_strTitle = "小房子浏览器在线升级";
			MsgBox.m_strPromptMessage = STRING(STR_PROMPT_NEWER_VERSION_AVAILABLE, "当前有更新版本的软件可用，是否立即升级？");
			// 默认使用静默方式升级
			MsgBox.m_bOption1 = TRUE;
			MsgBox.m_strOptionPromptMessage1 = STRING(STR_OPTION_UPGRADE_IN_BACKGROUND , "继续运行程序，从后台执行升级。");
			MsgBox.m_iType = MB_YESNO + MB_ICONQUESTION;
			if (IDOK == MsgBox.DoModal()) {
				// 先将本次选择的"是否静默升级"模式值传递出去
				m_bSilenceMode = MsgBox.m_bOption1;

				if (MsgBox.m_iID == IDYES) {
					// 进入升级，退出主程序，保证安全顺利升级
					CloseProgram(m_strAppName);
					return TRUE;
				} else {
					LOG(0, 0, "用户点击了取消按钮,无需升级,开始还原UpdateConfig.ini文件");
					RestoreUpdateCfgIniFile();
					return FALSE;
				}
			} else {
				LOG(0, 0, "用户关闭了升级提示窗口,无需升级,开始还原UpdateConfig.ini文件!");
				RestoreUpdateCfgIniFile();
				return FALSE;
			}
		}
	} else {
		LOG(0, 0, "检测到运行程序已是最新版本,无需升级!");
		RestoreUpdateCfgIniFile();
		if (!m_bSilenceMode) {
			// AfxMessageBox("当前已是最新版本,无需升级!");

			// 暂用“关于”窗口代替提示信息展现窗口
			CAboutDlg dlg;
			dlg.DoModal();
		}

		return FALSE;
	}

	return TRUE;
}

//
// 还原UpdateConfig.ini文件
//
BOOL CAutoUpdateApp::RestoreUpdateCfgIniFile()
{
	CString sConfigFilename = GetAppDirectory() + UPDATE_CONFIG_FILENAME;
	CString sBakConfigFilename = GetAppDirectory() + "Backup\\" + UPDATE_CONFIG_FILENAME;

	// 创建备份文件目录
	CreateDirectory((GetAppDirectory() + "Backup").GetBuffer(0), NULL);
	if (FALSE == CopyFile(sBakConfigFilename.GetBuffer(0),
		sConfigFilename.GetBuffer(0),
		FALSE)) {
		LOG(0, 0, "还原UpdateConfig.ini文件失败");
		return FALSE;
	}
	else {
		LOG(0, 0, "还原UpdateConfig.ini文件成功");
		return TRUE;
	}
}

//
// 从服务器下载升级配置文件，
// 检查是否有新版本可用
// 返回值为TRUE表示有新版本可用
//
BOOL CAutoUpdateApp::CheckUpdate()
{
	CString sConfigFilename = GetAppDirectory() + UPDATE_CONFIG_FILENAME;
	CString sBakConfigFilename = GetAppDirectory() + "Backup\\" + UPDATE_CONFIG_FILENAME;

	// 从指定的URL下载升级配置文件
	if (!m_strURL.IsEmpty()) {
		// 创建备份文件目录
		CreateDirectory((GetAppDirectory() + "Backup").GetBuffer(0), NULL);

		// 下载前，先备份UpdateConfig.ini文件
		if (FALSE == CopyFile(sConfigFilename.GetBuffer(0),
			sBakConfigFilename.GetBuffer(0),
			FALSE)) {
			LOG(0, 0, "备份UpdateConfig.ini文件失败");
			return FALSE;
		}

		if (Internet::InternetGetURL(m_strURL.GetBuffer(0), 
									sConfigFilename.GetBuffer(0)) 
			!= Internet::INTERNET_SUCCESS) {
			LOG(0, 0, "下载文件失败：%s", m_strURL.GetBuffer(0));
			return FALSE;
		}
	}

	// 从升级配置文件取得最新软件版本号
	CString sKey = "Version";
	const int BUFFER_SIZE = 512;
	char acBuffer[BUFFER_SIZE] = {0};
	GetPrivateProfileString(SECTION_UPDATE, 
							sKey.GetBuffer(0), 
							"", 
							acBuffer, 
							BUFFER_SIZE, 
							sConfigFilename.GetBuffer(0));
	CString strRemoteVersion = (char*)acBuffer;

	// 与当前软件版本号比较以确定是否需要升级
	if (strRemoteVersion > m_strVersion) {
		return TRUE;
	} else {
		return FALSE;
	}
}

//
// 初始化全局字符串常量表
//
void CAutoUpdateApp::InitStringTable(enum enLANGUAGE Language)
{
	if (Language < LANGUAGE_BOTTOM) {
		g_LanguageID = Language;
	} else {
		// 根据操作系统语言代码确定程序界面的语言代码
		switch (GetSystemDefaultLangID()) {
		case 0x0804: // Chinese (PRC)
		case 0x1004: // Chinese (Singapore)
			g_LanguageID = LANGUAGE_GB;
			break;
		case 0x0404: // Chinese (Taiwan)
		case 0x0c04: // Chinese (Hong Kong SAR, PRC)
		case 0x1404: // Chinese (Macao)
			g_LanguageID = LANGUAGE_BIG5;
			break;
		default:
			g_LanguageID = LANGUAGE_ENGLISH;
			break;
		}
	}

	//
	// 初始化全局字符串常量表
	//
	g_String[STR_NULL].Set("", "", "");
	g_String[STR_AUTO_UPDATE].Set("自动升级", "自動升級", "Auto Update");
	g_String[STR_APP_ALREADY_RUNNING].Set("升级程序已经在运行中！", "升級程序已經在運行中", "Auto update program is already running."); //  退出应用程序；提示保存文档\n退出

	g_String[STR_PROMPT_NEWER_VERSION_AVAILABLE].Set("当前有更新版本的软件可用，是否立即升级？", "當前有更新版本的軟件可用，是否立即升級？", "There is a newer version available. Do you want to update right now?");
	g_String[STR_OPTION_UPGRADE_IN_BACKGROUND].Set("后台执行升级", "後臺執行升級", "Run updating in background mode");
	g_String[STR_PROMPT_UPGRADE_READY].Set("升级准备就绪。", "升級準備就緒", "Update is ready. Please press [Start update] button to update.");
	g_String[STR_PROMPT_FAIL_TO_OPEN_UPDATE_CONFIG_FILE].Set("打开升级配置文件失败，无法执行升级！", "打開升級配置文件失敗，無法執行升級！", "Fail to open update config file. Update is canceled.");
	g_String[STR_PROMPT_DOWNLOADING_FILE].Set("正在下载文件 %s", "正在下載文件 %s", "Downloading file %s");
	g_String[STR_TOTAL_UPGRADE_PROGRESS].Set("升级总进度 %d / %d", "升級總進度 %d / %d", "Total progress %d / %d");
	g_String[STR_PROMPT_FAIL_IN_DOWNLOADING_FILES].Set("下载文件 %s 失败！", "下載文件 %s 失敗！", "Fail in downloading file %s!");
	g_String[STR_PROMPT_FAIL_IN_VERIFYING_FILES].Set("校验文件 %s 失败！", "校驗文件 %s 失敗！", "Fail in verifying file %s!");
	g_String[STR_PROMPT_FAIL_IN_UPDATING_FILES].Set("更新文件 %s 失败！", "更新文件 %s 失敗！", "Fail in updating file %s!");
	g_String[STR_PROMPT_FAIL_IN_UPDATING].Set("升级失败！", "升級失敗！", "Fail in updating!");
	g_String[STR_PROMPT_UPGRADE_FINISHED].Set("升级完毕！", "升級完畢！", "Update finished!");

	g_String[STR_BUTTON_START_UPGRADE].Set("开始升级", "開始升級", "Start update");
	g_String[STR_BUTTON_CANCEL_UPGRADE].Set("取消升级", "取消升級", "Cancel");
	g_String[STR_BUTTON_SUCCESS_UPGRADE].Set("完成升级", "完成升級", "Success update");

	g_String[STR_BUTTON_OK].Set("确定(&O)", "確定(&O)", "&OK");
	g_String[STR_BUTTON_CANCEL].Set("取消(&C)", "取消(&C)", "&Cancel");
	g_String[STR_BUTTON_ABORT].Set("跳出(&A)", "跳出(&A)", "&Abort");
	g_String[STR_BUTTON_IGANORE].Set("忽略(&I)", "忽略(&I)", "&Ignore");
	g_String[STR_BUTTON_RETRY].Set("重试(&R)", "重試(&R)", "&Retry");
	g_String[STR_BUTTON_CONTINUE].Set("继续(&C)", "繼續(&C)", "&Continue");
	g_String[STR_BUTTON_YES].Set("是(&Y)", "是(&Y)", "&Yes");
	g_String[STR_BUTTON_NO].Set("否(&N)", "否(&N)", "&No");
	g_String[STR_BUTTON_CLOSE].Set("关闭", "關閉", "Close");
	g_String[STR_BUTTON_APPLY].Set("应用(&A)", "應用(&A)", "&Apply");

	g_String[STR_OTHER].Set("其他", "其他", "Other");

	g_String[STR_ERROR].Set("错误", "錯誤", "Error");
	g_String[STR_ERROR_MESSAGE].Set("错误：%s", "錯誤：%s", "Error: %s");
}

BOOL CAutoUpdateApp::IsAppRunning()
{
	// 创建互斥量，防止同时启动多个程序实例
	CString sMutex = m_strAppName + "AutoUpdateMutex";
	if (::OpenMutex(MUTEX_ALL_ACCESS, FALSE, sMutex.GetBuffer(0))) {
		return TRUE;
	} else {
		::CreateMutex(0, FALSE, sMutex.GetBuffer(0));
		return FALSE;
	}
}

//
// 终止MyBrowser.exe程序
//
void CAutoUpdateApp::CloseProgram(CString strProgram)
{
	// 定义CreateToolhelp32Snapshot系统快照句柄 
	HANDLE handle;
	// 定义要结束进程句柄 
	HANDLE handle1;
	// 获得系统快照句柄 
	handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	// 定义PROCESSENTRY32结构字指针
	PROCESSENTRY32 *info; 
	// PROCESSENTRY32 结构的 dwSize 成员设置成 sizeof(PROCESSENTRY32) 

	info = new PROCESSENTRY32; 
	info->dwSize = sizeof(PROCESSENTRY32); 
	// 调用一次 Process32First 函数，从快照中获取进程列表 
	Process32First(handle, info); 
	// 重复调用 Process32Next，直到函数返回 FALSE 为止 
	while(Process32Next(handle, info) != FALSE) {
		// 指向进程名字
		CString strTmp = info->szExeFile;

		if (strProgram.CompareNoCase(info->szExeFile) == 0 ) {   
			// PROCESS_TERMINATE表示为结束操作打开,
			// FALSE=可继承,info->th32ProcessID=进程ID    
			handle1 = OpenProcess(PROCESS_TERMINATE, FALSE, info->th32ProcessID); 
			// 结束进程    
			TerminateProcess(handle1, 0);    
		}   
	}
	delete info;

	CloseHandle(handle);
}