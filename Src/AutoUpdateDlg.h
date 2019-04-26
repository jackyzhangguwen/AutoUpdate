// AutoUpdateDlg.h : header file
//

#if !defined(AFX_AUTOUPDATEDLG_H__4CA1BB40_75FF_4A00_AF96_F141507CE885__INCLUDED_)
#define AFX_AUTOUPDATEDLG_H__4CA1BB40_75FF_4A00_AF96_F141507CE885__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CAutoUpdateDlg dialog

#include "RDialog.h"

class CUpdateThread;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	// DDX/DDV support
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	//}}AFX_VIRTUAL

	// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CAutoUpdateDlg : public CRDialog
{
// Construction
public:
	// standard constructor
	CAutoUpdateDlg(CWnd* pParent = NULL);
	// 设置配置文件名
	BOOL SetConfigFile(CString& strFilename);
	BOOL DoUpdate(void);
	LRESULT OnUserMessage(WPARAM wParam, LPARAM lParam);
	void UpdateProgress(UINT iTotalFileCount, UINT iFinishedFileCount, float fTotalPercent, float fPercent);
	void OnNotifyUpdateFinish(BOOL bSuccess = TRUE);

// Dialog Data
	//{{AFX_DATA(CAutoUpdateDlg)
	enum { IDD = IDD_AUTOUPDATE };
	CRButton	m_btnUpgrade;
	CRButton	m_btnCancel;
	// 应用程序名
	CString m_strAppName;
	// 下载文件进度提示信息
	CString	m_strPrompt;
	// 升级总进度提示信息
	CString	m_strPromptTotalProgress;
	// CString m_strStatus;
	// 下载文件进度条
	CString	m_strProgressBar;
	// 升级总进度条
	CString	m_strTotalProgressBar;
	// 静默方式执行升级，不显示升级程序界面，
	// 只在升级完毕后提醒用户
	BOOL m_bSilenceMode;
	// 用户终止升级
	BOOL m_bUserBreak;
	// 执行升级的线程
	CUpdateThread *m_pUpdateThread;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAutoUpdateDlg)
	protected:
	// DDX/DDV support
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// 升级配置文件名
	CString m_strConfigFileName;
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CAutoUpdateDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// 要下载的文件总数和大小总和
	DOWNLOAD_INFO_STRU m_TotalFileInfo;
	// 已下载的文件总数和大小总和
	DOWNLOAD_INFO_STRU m_FinishedFileInfo;
	// 当前正在下载的文件进度
	float m_fDownloadPercent;
	// 所有要下载的文件总进度
	float m_fTotalDownloadPercent;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUTOUPDATEDLG_H__4CA1BB40_75FF_4A00_AF96_F141507CE885__INCLUDED_)
