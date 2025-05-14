
// TlPrizeSenderDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "TlPrizeSender.h"
#include "TlPrizeSenderDlg.h"
#include "afxdialogex.h"
#include "CDatabaseSettingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <stdexcept>
#include "dbAction.h"


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

// CTlPrizeSenderDlg 对话框



CTlPrizeSenderDlg::CTlPrizeSenderDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TLPRIZESENDER_DIALOG, pParent)
	, m_username(_T(""))
	, m_world(0)
	, m_charguid(0)
	, m_item(0)
	, m_count(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTlPrizeSenderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_username);
	DDX_Text(pDX, IDC_EDIT2, m_world);
	DDX_Text(pDX, IDC_EDIT3, m_charguid);
	DDX_Text(pDX, IDC_EDIT4, m_item);
	DDX_Text(pDX, IDC_EDIT5, m_count);
	DDV_UsernameNotEmpty(pDX, m_username);
	DDV_MinMaxInt(pDX, m_item, 1, INT_MAX);
	DDV_MinMaxInt(pDX, m_count, 1, INT_MAX);
	DDX_Control(pDX, IDC_BUTTON1, m_sendButton);
}

BEGIN_MESSAGE_MAP(CTlPrizeSenderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_DB_SETTING, &CTlPrizeSenderDlg::OnShowSettingDlg)
	ON_COMMAND(ID_APP_ABOUT, &CTlPrizeSenderDlg::OnAppAbout)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON1, &CTlPrizeSenderDlg::OnSendBnClicked)
END_MESSAGE_MAP()


// CTlPrizeSenderDlg 消息处理程序

BOOL CTlPrizeSenderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_appConfig.loadFromFile(APP_CONFIG_PATH);
	m_username = m_appConfig.m_prizeConfig.m_username;
	m_world = m_appConfig.m_prizeConfig.m_world;
	m_charguid = m_appConfig.m_prizeConfig.m_charguid;
	m_item = m_appConfig.m_prizeConfig.m_item;;
	m_count = m_appConfig.m_prizeConfig.m_count;
	UpdateData(false);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void AFXAPI CTlPrizeSenderDlg::DDV_UsernameNotEmpty(CDataExchange* pDX, CString const& value)
{
	if (pDX->m_bSaveAndValidate && value.GetLength() == 0)
	{
		CString prompt = _T("游戏账号不能为空");
		AfxMessageBox(prompt, MB_ICONEXCLAMATION, AFX_IDP_PARSE_STRING_SIZE);
		prompt.Empty(); // exception prep
		pDX->Fail();
	}
}

void CTlPrizeSenderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		OnAppAbout();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTlPrizeSenderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTlPrizeSenderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTlPrizeSenderDlg::OnShowSettingDlg()
{
	CDatabaseSettingDlg dlg;
	//get
	dlg.m_host = m_appConfig.m_dbConfig.m_host;
	dlg.m_port = m_appConfig.m_dbConfig.m_port;
	dlg.m_name = m_appConfig.m_dbConfig.m_name;
	dlg.m_username = m_appConfig.m_dbConfig.m_username;
	dlg.m_password = m_appConfig.m_dbConfig.m_password;
	//set
	if (dlg.DoModal() == IDOK) {
		m_appConfig.m_dbConfig.m_host = dlg.m_host;
		m_appConfig.m_dbConfig.m_port = dlg.m_port;
		m_appConfig.m_dbConfig.m_name = dlg.m_name;
		m_appConfig.m_dbConfig.m_username = dlg.m_username;
		m_appConfig.m_dbConfig.m_password = dlg.m_password;
	}
}

void CTlPrizeSenderDlg::OnAppAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}


void CTlPrizeSenderDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	m_appConfig.saveToFile(APP_CONFIG_PATH);
}


void CTlPrizeSenderDlg::OnSendBnClicked()
{
	if (!UpdateData(true)) {
		return;
	}
	//记录发放配置
	m_appConfig.m_prizeConfig.m_username = m_username;
	m_appConfig.m_prizeConfig.m_world = m_world;
	m_appConfig.m_prizeConfig.m_charguid = m_charguid;
	m_appConfig.m_prizeConfig.m_item = m_item;;
	m_appConfig.m_prizeConfig.m_count = m_count;
	CString btn_text;
	m_sendButton.GetWindowText(btn_text);
	m_sendButton.EnableWindow(false);
	m_sendButton.SetWindowText(_T("发放中..."));
	bool sendSuccess = true;
	try {
		processSendPrize(m_appConfig.m_dbConfig,m_username, m_world, m_charguid, m_item, m_count);
	}
	catch (std::runtime_error& err) {
		sendSuccess = false;
		CString errMessage = _T("发送活动奖励失败: ");
		errMessage += err.what();
		MessageBox(errMessage, _T("出错了"), MB_ICONSTOP);
	}
	m_sendButton.EnableWindow(true);
	m_sendButton.SetWindowText(btn_text);
	if (sendSuccess) {
		MessageBox(_T("发送活动奖励成功"),_T("提示"), MB_ICONINFORMATION);
	}
}
