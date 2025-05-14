// CDatabaseSettingDlg.cpp: 实现文件
//

#include "pch.h"
#include "TlPrizeSender.h"
#include "afxdialogex.h"
#include "CDatabaseSettingDlg.h"


// CDatabaseSettingDlg 对话框

IMPLEMENT_DYNAMIC(CDatabaseSettingDlg, CDialogEx)

CDatabaseSettingDlg::CDatabaseSettingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_DB_SETTING, pParent)
	, m_host(_T(""))
	, m_port(0)
	, m_username(_T(""))
	, m_password(_T(""))
	, m_name(_T(""))
{

}

CDatabaseSettingDlg::~CDatabaseSettingDlg()
{
}

void CDatabaseSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_host);
	DDX_Text(pDX, IDC_EDIT2, m_port);
	DDV_MinMaxInt(pDX, m_port, 1, 65535);
	DDX_Text(pDX, IDC_EDIT5, m_name);
	DDX_Text(pDX, IDC_EDIT3, m_username);
	DDX_Text(pDX, IDC_EDIT4, m_password);
}


BEGIN_MESSAGE_MAP(CDatabaseSettingDlg, CDialogEx)
END_MESSAGE_MAP()


// CDatabaseSettingDlg 消息处理程序
