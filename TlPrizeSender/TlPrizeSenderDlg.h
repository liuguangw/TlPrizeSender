
// TlPrizeSenderDlg.h: 头文件
//

#pragma once
#include "CAppConfig.h"


// CTlPrizeSenderDlg 对话框
class CTlPrizeSenderDlg : public CDialogEx
{
	// 构造
public:
	CTlPrizeSenderDlg(CWnd* pParent = nullptr);	// 标准构造函数

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TLPRIZESENDER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


	// 实现
protected:
	// 配置文件路径
	const LPCTSTR APP_CONFIG_PATH = _T("./config/config.ini");
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	// 校验用户名是否为空
	void AFXAPI DDV_UsernameNotEmpty(CDataExchange* pDX, CString const& value);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnShowSettingDlg();
	afx_msg void OnAppAbout();
	afx_msg void OnDestroy();
	afx_msg void OnSendBnClicked();
private:
	CAppConfig m_appConfig;
	CString m_username;
	int m_world;
	int m_charguid;
	int m_item;
	int m_count;
	CButton m_sendButton;
};
