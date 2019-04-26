//------------------------------------------------------------------------------
// 文件名称：RMessageBox.h
// 文件版本：v1.0
// 文件描述：自定义消息框类
//------------------------------------------------------------------------------

#pragma once

#ifndef RMESSAGEBOX_H
#define RMESSAGEBOX_H

#include "RDialog.h"
#include "RButton.h"

// CRMessageBox 对话框

class CRMessageBox : public CRDialog
{
	DECLARE_DYNAMIC(CRMessageBox)

public:
	// 标准构造函数
	CRMessageBox(CWnd* pParent = NULL);
	virtual ~CRMessageBox();

	// 对话框数据
	enum { IDD = IDD_MESSAGEBOX };

protected:
	// DDX/DDV 支持
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

protected:
	afx_msg void OnPaint();
	afx_msg void OnBtnClickedOk();
	afx_msg void OnBtnClickedCancel();
	afx_msg void OnBtnClickedAbort();
	afx_msg void OnBtnClickedIganore();
	afx_msg void OnBtnClickedRetry();
	afx_msg void OnBtnClickedContinue();
	afx_msg void OnBtnClickedYes();
	afx_msg void OnBtnClickedNo();
	void ResetMessageBox();

public:
	// 消息框标题
	CString m_strTitle;
	// 消息框样式
	int m_iType;
	// 用户点中的按钮
	int m_iID;
	// 提示信息
	CString m_strPromptMessage;
	// 复选框1提示信息，当此字符串不为空时，显示复选框，否则隐藏复选框
	CString m_strOptionPromptMessage1;
	// 复选框1选则状态的值
	BOOL m_bOption1;
	// 复选框2提示信息，当此字符串不为空时，显示复选框，否则隐藏复选框
	CString m_strOptionPromptMessage2;
	// 复选框2选则状态的值
	BOOL m_bOption2;

protected:
	CStatic m_stcIcon;
	CStatic m_stcPromptMessage;
	CButton m_chkOption1;
	CButton m_chkOption2;
	CRButton m_btnOK;
	CRButton m_btnCancel;
	CRButton m_btnAbort;
	CRButton m_btnIganore;
	CRButton m_btnRetry;
	CRButton m_btnContinue;
	CRButton m_btnYes;
	CRButton m_btnNo;
};

#endif // #ifndef RMESSAGEBOX_H
