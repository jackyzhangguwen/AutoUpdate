// AutoUpdate.h : main header file for the AUTOUPDATE application
//

#if !defined(AFX_AUTOUPDATE_H__187D3C3A_DD05_4283_9FCA_1FAF58B23B29__INCLUDED_)
#define AFX_AUTOUPDATE_H__187D3C3A_DD05_4283_9FCA_1FAF58B23B29__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

// main symbols
#include "resource.h"

#include "MultiLanguage.h"

// 本地保存的升级配置文件名
#define UPDATE_CONFIG_FILENAME "UpdateConfig.ini"

// 升级配置文件中的区段名
#define SECTION_UPDATE "UPDATE"
#define SECTION_COMMON "COMMON"

// 上报到界面的升级进度消息
enum
{
	// 通知要下载的文件状况
	NOTIFY_DOWNLOAD_INFO = 1001,
	// 通知已下载的文件状况
	NOTIFY_DOWNLOADED_INFO,
	// 通知下载单个文件进度
	NOTIFY_DOWNLOAD_PROGRESS,
	// 通知正在下载的文件名
	NOTIFY_DOWNLOADING_FILENAME,
	// 通知下载文件失败
	NOTIFY_DOWNLOAD_FILE_FAIL,
	// 通知校验文件失败
	NOTIFY_VERIFY_FILE_FAIL,
	// 通知更新文件失败
	NOTIFY_UPDATE_FILE_FAIL,
	// 通知升级完毕消息
    NOTIFY_FINISH_UPDATE,
};

// 升级下载文件状态消息结构
struct DOWNLOAD_INFO_STRU
{
	// 合计文件数
	UINT iFileCount;
	// 合计字节数
	UINT iFileSize;
};

// 下载单个文件进度消息结构
struct DOWNLOAD_PROGRESS_STRU
{
	// 当前正在下载的文件的字节数
	UINT iCurrentFileSize;
										
	// 当前文件已下载字节数
	UINT iCurrentFileFinishedSize;
};

/////////////////////////////////////////////////////////////////////////////
// CAutoUpdateApp:
// See AutoUpdate.cpp for the implementation of this class
//

class CAutoUpdateApp : public CWinApp
{
public:
	CAutoUpdateApp();
	void InitStringTable(enum enLANGUAGE Language = LANGUAGE_BOTTOM);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAutoUpdateApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAutoUpdateApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public :
	// 下载升级基础URL
	CString m_strBaseURL;

private:
	// 处理命令行
	BOOL ParseCommandLine();

	// 判断本程序是否正在运行
	BOOL IsAppRunning();

	// 从软件网站下载升级配置文件，
	// 检查是否有新版本的软件可用
	BOOL CheckUpdate();
	
	// 终止主程序（例如：小房子浏览器程序MyBrowser.exe）
	void CloseProgram(CString strProgram);

	// 还原UpdateConfig.ini文件
	BOOL RestoreUpdateCfgIniFile();

private:
	// 应用程序名，用以创建互斥量
	CString m_strAppName;
								
	// 应用程序的当前版本
	CString m_strVersion;

	// 下载升级配置文件的URL
	CString m_strURL;

	// 升级过程中发送消息的目标窗口句柄
	// DWORD m_iNotifyWindow;
	
	// 升级过程中发送消息的目标窗口标题
	// CString m_strNotifyWindowTitle;
										
    // 升级完毕发送的消息代码
	// DWORD m_iNotifyFinish;

	// 静默方式执行升级，不显示升级程序界面，
	// 只在升级完毕后提醒用户
	BOOL m_bSilenceMode;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUTOUPDATE_H__187D3C3A_DD05_4283_9FCA_1FAF58B23B29__INCLUDED_)
