#pragma once
#include "afxdialogex.h"


// CDatabaseSettingDlg 对话框

class CDatabaseSettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDatabaseSettingDlg)

public:
	CDatabaseSettingDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDatabaseSettingDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DB_SETTING };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_host;
	int m_port;
	CString m_name;
	CString m_username;
	CString m_password;
};
