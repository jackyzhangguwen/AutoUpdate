// UpdateThread.cpp : implementation file
//

#include "stdafx.h"
#include "AutoUpdate.h"
#include "InternetGetFile.h"
#include "UpdateThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUpdateThread

IMPLEMENT_DYNCREATE(CUpdateThread, CWinThread)

CUpdateThread::CUpdateThread()
{
	m_bSilenceMode = FALSE;
	m_bUserBreak = FALSE;
	m_fPercent = 0;
	m_hProgressWindow = NULL;
}

CUpdateThread::~CUpdateThread()
{
}

BOOL CUpdateThread::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int CUpdateThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	m_bUserBreak = TRUE;

	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CUpdateThread, CWinThread)
	//{{AFX_MSG_MAP(CUpdateThread)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateThread message handlers

int CUpdateThread::Run() 
{
	LOG(0, 0, "更新线程开始工作...");

	try {
		if (DoUpdate()) {
			if (m_hProgressWindow) {
				SendMessage(m_hProgressWindow, WM_USER, (WPARAM)NOTIFY_FINISH_UPDATE, (LPARAM)1);
			}

			LOG(0, 0, "更新线程成功完成!");
			ExitThread(1);
			return 1;
		} else {
			if (m_hProgressWindow) {
				SendMessage(m_hProgressWindow, WM_USER, (WPARAM)NOTIFY_FINISH_UPDATE, (LPARAM)0);
			}

			LOG(0, 0, "更新线程失败结束!");

			ExitThread(0);
			return 0;
		}
	} catch (...) {
		LOG(0, 0, "更新线程出现异常！");
		ExitThread(0);
	}
	//	return CWinThread::Run();
}

//
// 执行升级
//
BOOL CUpdateThread::DoUpdate()
{
	if (m_strConfigFileName.IsEmpty()) { return FALSE; }

	UINT iCommonFileCount = 0;
	DOWNLOAD_INFO_STRU DownloadInfo = {0};
	UINT i;
	const int BUFFER_SIZE = 512;
	char acBuffer[BUFFER_SIZE];
	CString sSection;
	CString sKey;
	CString strFileName;
	CString sHash;
	CString sURL;
	CString sTemp;

	// 创建保存升级文件的目录
	CreateDirectory(GetAppDirectory() + "Update", NULL);

	//
	// 取得公共文件数及文件大小总和
	//
	LOG(0, 0, "开始获取本次升级涉及的文件总个数和总大小...");
	iCommonFileCount = GetPrivateProfileInt(SECTION_COMMON,
											"FileCount", 
											0,
											m_strConfigFileName.GetBuffer(0));
	for (i = 1; i <= iCommonFileCount; i++) {
		sSection.Format("CommonFile%d", i);
		DownloadInfo.iFileSize += GetPrivateProfileInt(sSection.GetBuffer(0), 
														"Size",
														0,
														m_strConfigFileName.GetBuffer(0));
	}
	DownloadInfo.iFileCount += iCommonFileCount;

	// 将文件总数和文件大小总和上报到界面
	if (!m_bSilenceMode && 
		m_hProgressWindow != NULL) {
		SendMessage(m_hProgressWindow, 
					WM_USER, 
					(WPARAM)NOTIFY_DOWNLOAD_INFO, 
					(LPARAM)&DownloadInfo);
	}

	LOG(0, 0, "文件总个数为：%u (%u BYTES).", DownloadInfo.iFileCount, DownloadInfo.iFileSize);

	LOG(0, 0, "获取升级文件总个数和总大小完成!");

	//
	// 下载所有文件
	//

	LOG(0, 0, "开始下载文件...");

	memset(&DownloadInfo, 0, sizeof(DOWNLOAD_INFO_STRU));

	// 循环下载所有公共文件
	for (i = 1; i <= iCommonFileCount; i++) {
		if (m_bUserBreak) {
			return FALSE;
		}

		sSection.Format("CommonFile%d", i);
		if (!DownloadFile(sSection)) {
			// 文件下载失败，升级失败
			if (!m_bSilenceMode && 
				m_hProgressWindow != NULL) {
				sKey = "Name";
				memset(acBuffer, 0, BUFFER_SIZE);
				GetPrivateProfileString(sSection.GetBuffer(0),
										sKey.GetBuffer(0), 
										"",
										(char*)acBuffer, 
										BUFFER_SIZE, 
										m_strConfigFileName.GetBuffer(0));
				SendMessage(m_hProgressWindow, 
							WM_USER, 
							(WPARAM)NOTIFY_DOWNLOAD_FILE_FAIL, 
							(LPARAM)acBuffer);
			}
			return FALSE;
		}

		// 将升级进度上报到界面
		DownloadInfo.iFileCount++;
		DownloadInfo.iFileSize += GetPrivateProfileInt(sSection.GetBuffer(0), 
														"Size", 
														0, 
														m_strConfigFileName.GetBuffer(0));
		if (!m_bSilenceMode && 
			m_hProgressWindow != NULL) {
			SendMessage(m_hProgressWindow, 
						WM_USER, 
						(WPARAM)NOTIFY_DOWNLOADED_INFO, 
						(LPARAM)&DownloadInfo);
		}

		LOG(0, 0, "当前共下载成功文件数为：%u (%u BYTES).", DownloadInfo.iFileCount, DownloadInfo.iFileSize);
	}

	LOG(0, 0, "文件下载完成!");

	//
	// 下载完毕后校验文件
	//

	LOG(0, 0, "开始校验文件大小和MD5签名...");

	// 循环校验所有公共文件
	for (i = 1; i <= iCommonFileCount; i++) {
		if (m_bUserBreak) {
			return FALSE;
		}

		sSection.Format("CommonFile%d", i);

		if (!VerifyFile(sSection)) {
			// 文件校验不通过，升级失败
			if (!m_bSilenceMode && 
				m_hProgressWindow != NULL) {
				sKey = "Name";
				memset(acBuffer, 0, BUFFER_SIZE);
				GetPrivateProfileString(sSection.GetBuffer(0), 
										sKey.GetBuffer(0), 
										"",
										(char*)acBuffer, 
										BUFFER_SIZE, 
										m_strConfigFileName.GetBuffer(0));
				SendMessage(m_hProgressWindow, 
							WM_USER, 
							(WPARAM)NOTIFY_VERIFY_FILE_FAIL, 
							(LPARAM)acBuffer);
			}
			return FALSE;
		}
	}

	LOG(0, 0, "文件校验完成!");

	//
	// 复制、更新文件
	//

	LOG(0, 0, "开始覆盖更新文件...");

	// 创建备份文件目录
	CreateDirectory((GetAppDirectory() + "Backup").GetBuffer(0), NULL);
	// 循环更新所有文件
	for (i = 1; i <= iCommonFileCount; i++) {
		if (m_bUserBreak) {
			return FALSE;
		}

		sSection.Format("CommonFile%d", i);
		if (!UpdateFile(sSection)) {
			// 文件更新不成功，升级失败
			if (!m_bSilenceMode && 
				m_hProgressWindow != NULL) {
				sKey = "Name";
				memset(acBuffer, 0, BUFFER_SIZE);
				GetPrivateProfileString(sSection.GetBuffer(0),
										sKey.GetBuffer(0), 
										"",
										(char*)acBuffer, 
										BUFFER_SIZE, 
										m_strConfigFileName.GetBuffer(0));
				SendMessage(m_hProgressWindow, 
							WM_USER, 
							(WPARAM)NOTIFY_UPDATE_FILE_FAIL, 
							(LPARAM)acBuffer);
			}
			return FALSE;
		}
	}

	LOG(0, 0, "覆盖更新文件完成!");

	//
	// 执行升级程序
	//
	LOG(0, 0, "开始执行更新后程序...");

	sKey = "RunAfterDownload";
	memset(acBuffer, 0, BUFFER_SIZE);
	GetPrivateProfileString(SECTION_UPDATE, 
							sKey.GetBuffer(0), 
							"",
							(char*)acBuffer, 
							BUFFER_SIZE, 
							m_strConfigFileName.GetBuffer(0));
	strFileName = (char*)acBuffer;

	if (!strFileName.IsEmpty()) {
		LOG(0, 0, "开始启动：%s", strFileName.GetBuffer(0));

		ShellExecute(NULL, 
					"open", 
					(GetAppDirectory() + strFileName).GetBuffer(0),
					"", 
					"", 
					SW_NORMAL);
	}
	LOG(0, 0, "执行更新后程序完成!");

	LOG(0, 0, "升级工作整体完成!");

	// 通知界面升级完毕
	if (!m_bSilenceMode && 
		m_hProgressWindow != NULL) {
		SendMessage(m_hProgressWindow, 
					WM_USER, 
					(WPARAM)NOTIFY_FINISH_UPDATE, 
					0);
	}

	return TRUE;
}

// 检查并下载一个文件
BOOL CUpdateThread::DownloadFile(CString &strFileSection)
{
	const int BUFFER_SIZE = 512;
	char acBuffer[BUFFER_SIZE];
	CString strFileName;
	CString strHash;
	CString strURL;
	CString strKey;

	LOG(0, 0, "开始处理下载文件配置节：%s", strFileSection.GetBuffer(0));

	// 比较文件是否已经下载了，如是则跳过
	if (VerifyFile(strFileSection)) {
		LOG(0, 0, "本地文件%s已是最新,无需下载.", strFileName.GetBuffer(0));
		return TRUE;
	}

	// 取得文件名
	strKey = "Name";
	memset(acBuffer, 0, BUFFER_SIZE);
	GetPrivateProfileString(strFileSection.GetBuffer(0),
		strKey.GetBuffer(0),
		"",
		(char*)acBuffer,
		BUFFER_SIZE,
		m_strConfigFileName.GetBuffer(0));
	strFileName = (char*)acBuffer;

	LOG(0, 0, "开始通过网络下载文件：%s.", strFileName.GetBuffer(0));

	// // 取得文件URL
	// strKey = "URL";
	// memset(acBuffer, 0, BUFFER_SIZE);
	// GetPrivateProfileString(strFileSection.GetBuffer(0),
	//						strKey.GetBuffer(0),
	//						"",
	//						(char*)acBuffer, 
	//						BUFFER_SIZE, 
	//						m_strConfigFileName.GetBuffer(0));
	// strURL = (char*)acBuffer;

	CAutoUpdateApp* pMainApp = (CAutoUpdateApp*)::AfxGetApp();
	strURL = pMainApp->m_strBaseURL + "/" + strFileName;

	// 更新显示正在下载的文件
	// 下载文件
	if (!m_bSilenceMode && 
		m_hProgressWindow != NULL) {
		SendMessage(m_hProgressWindow, 
					WM_USER, 
					(WPARAM)NOTIFY_DOWNLOADING_FILENAME,
					(LPARAM)strFileName.GetBuffer(0));
	}
	if (Internet::InternetGetURL(strURL.GetBuffer(0), 
								(GetAppDirectory() + "Update\\" + strFileName).GetBuffer(0),
								NULL,
								m_hProgressWindow)
		!= Internet::INTERNET_SUCCESS) {
		// 记录下载文件失败日志
		LOG(0, 0, "下载文件失败：%s", strFileName.GetBuffer(0));

		LOG(0, 0, "文件下载URL为：%s", strURL.GetBuffer(0));

		return FALSE;
	} else {
		// 记录下载文件成功日志
		LOG(0, 0, "下载文件成功：%s", strFileName.GetBuffer(0));
	}

	return TRUE;
}

// 校验文件(update目录下文件与服务器相应文件做校验)
BOOL CUpdateThread::VerifyFile(CString &strFileSection)
{
	const int BUFFER_SIZE = 512;
	char acBuffer[BUFFER_SIZE];
	CString strFileName;
	UINT iFileSize;
	CString sHash;
	CString sKey;

	LOG(0, 0, "开始校验文件配置节 = %s.", strFileSection.GetBuffer(0));

	// 取得文件名
	sKey = "Name";
	memset(acBuffer, 0, BUFFER_SIZE);
	GetPrivateProfileString(strFileSection.GetBuffer(0),
							sKey.GetBuffer(0), 
							"",
							(char*)acBuffer, 
							BUFFER_SIZE, 
							m_strConfigFileName.GetBuffer(0));
	strFileName = (char*)acBuffer;

	// 取得文件大小
	sKey = "Size";
	iFileSize = GetPrivateProfileInt(strFileSection.GetBuffer(0), 
									sKey.GetBuffer(0),
									0,
									m_strConfigFileName.GetBuffer(0));
	UINT iLocalFileSize = GetFileSize(GetAppDirectory() + "Update\\" + strFileName);
	if (iLocalFileSize == iFileSize) {
		sKey = "Hash";
		memset(acBuffer, 0, BUFFER_SIZE);
		GetPrivateProfileString(strFileSection.GetBuffer(0), 
								sKey.GetBuffer(0),
								"",
								(char*)acBuffer, 
								BUFFER_SIZE, 
								m_strConfigFileName.GetBuffer(0));
		sHash = (char*)acBuffer;

		// 计算文件的Hash码以进行比较
		unsigned char acMD5Digest[16];
		CalculateMD5((GetAppDirectory() + "Update\\" + strFileName).GetBuffer(0), acMD5Digest);

		if (sHash.CompareNoCase(MD5toString(acMD5Digest)) == 0) {
			return TRUE;
		} else {
			LOG(0, 0, "校验文件MD5失败：%s，本地文件MD5：%s，预期文件MD5为：%s", 
				strFileName.GetBuffer(0),
				sHash.GetBuffer(0),
				MD5toString(acMD5Digest).GetBuffer(0));
		}
	} else {
		LOG(0, 0, "校验文件大小失败：%s，本地文件大小：%u，预期大小为：%u", 
			strFileName.GetBuffer(0),
			iLocalFileSize, 
			iFileSize);
	}

	return FALSE;
}

// 更新文件
BOOL CUpdateThread::UpdateFile(CString &strFileSection)
{
	const int BUFFER_SIZE = 512;
	char acBuffer[BUFFER_SIZE];
	CString strFileName;
	CString sFileSubcatalog;
	CString sDestFilename;
	CString sBackupFilename;
	CString sKey;

	// 取得文件名
	sKey = "Name";
	memset(acBuffer, 0, BUFFER_SIZE);
	GetPrivateProfileString(strFileSection.GetBuffer(0), 
							sKey.GetBuffer(0),
							"",
							(char*)acBuffer, 
							BUFFER_SIZE, 
							m_strConfigFileName.GetBuffer(0));
	strFileName = (char*)acBuffer;

	// 取得子目录结构
	sKey = "Subcatalog";
	memset(acBuffer, 0, BUFFER_SIZE);
	GetPrivateProfileString(strFileSection.GetBuffer(0),
							sKey.GetBuffer(0),
							"",
							(char*)acBuffer, 
							BUFFER_SIZE, 
							m_strConfigFileName.GetBuffer(0));
	sFileSubcatalog = (char*)acBuffer;

	// 取得目标文件名
	sKey = "DestPath";
	memset(acBuffer, 0, BUFFER_SIZE);
	GetPrivateProfileString(strFileSection.GetBuffer(0), 
							sKey.GetBuffer(0), 
							"",
							(char*)acBuffer, 
							BUFFER_SIZE, 
							m_strConfigFileName.GetBuffer(0));
	sDestFilename = (char*)acBuffer;
	sDestFilename.TrimLeft();
	sDestFilename.TrimRight();
	if (sDestFilename.IsEmpty()) {
		// 无目标目录，文件无需复制
		return TRUE;
	}
	sDestFilename += sFileSubcatalog + strFileName;

	// 替换变量字符串为系统变量
	sDestFilename = ResolvePath(sDestFilename.GetBuffer(0));

	// 备份原文件
	sBackupFilename = GetAppDirectory() + "Backup\\" + strFileName;
	if (GetFileSize(sDestFilename) > 0) {
		char acBuffer[MAX_PATH] = {0};

		// 取得本自动升级程序的文件全路径
		GetModuleFileName(AfxGetInstanceHandle(), acBuffer, sizeof(acBuffer));

		if (strFileName.CompareNoCase(GetFilename(acBuffer)) == 0) {
			// 要更新的文件是本自动升级程序，须换名更新
			CopyFile(sDestFilename.GetBuffer(0), sBackupFilename.GetBuffer(0), FALSE);
			// 复制新文件，新文件名加上 .new 后缀，由主程序来将其更名
			sDestFilename += ".new";
			return CopyFile((GetAppDirectory() + "Update\\" + strFileName).GetBuffer(0), 
							sDestFilename.GetBuffer(0), 
							FALSE);
		} else {
			MoveFile(sDestFilename.GetBuffer(0), sBackupFilename.GetBuffer(0));
		}
	}

	// 如果输出目录在子目录中，则创建对应的目录结构
	if (sFileSubcatalog!="") {
		CString csTemp = sFileSubcatalog.Left(strlen(sFileSubcatalog) - 1);
		CreateDirectory(GetAppDirectory() + csTemp, NULL);
	}

	// 复制新文件
	return CopyFile((GetAppDirectory() + "Update\\" + strFileName).GetBuffer(0), 
					sDestFilename.GetBuffer(0), 
					FALSE);
}
