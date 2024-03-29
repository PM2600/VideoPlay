
// VideoClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "VideoClient.h"
#include "VideoClientDlg.h"
#include "afxdialogex.h"
#include "VideoClientController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVideoClientDlg 对话框



CVideoClientDlg::CVideoClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VIDEOCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_status = false;
	m_length = 0.0f;
}

void CVideoClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PLAY, m_video);
	DDX_Control(pDX, IDC_SLIDER_POS, m_pos);
	DDX_Control(pDX, IDC_SLIDER_VOLUME, m_volume);
	DDX_Control(pDX, IDC_EDIT_URL, m_url);
	DDX_Control(pDX, IDC_BTN_PLAY, m_btnPlay);
}

BEGIN_MESSAGE_MAP(CVideoClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_PLAY, &CVideoClientDlg::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_STOP, &CVideoClientDlg::OnBnClickedBtnStop)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER_POS, &CVideoClientDlg::OnTRBNThumbPosChangingSliderPos)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER_VOLUME, &CVideoClientDlg::OnTRBNThumbPosChangingSliderVolume)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()


// CVideoClientDlg 消息处理程序

BOOL CVideoClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	SetTimer(0, 500, NULL);
	m_pos.SetRange(0, 1);
	m_volume.SetRange(0, 100);
	m_volume.SetTicFreq(20);
	SetDlgItemText(IDC_STATIC_VOLUME, _T("100%"));
	SetDlgItemText(IDC_STATIC_TIME, _T("--:--:-- / --:--:--"));
	m_controller->SetWnd(m_video.GetSafeHwnd());
	m_url.SetWindowText(_T("file:///D:\\2.5\\Github\\VideoPlay\\VideoPlay\\VideoClient\\StreetFighter6.mp4"));
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CVideoClientDlg::OnPaint()
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
HCURSOR CVideoClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CVideoClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0) {
		//更新音量
		//更新播放时间
		float pos = m_controller->VideoCtrl(EVLC_GET_POSITION);
		if (pos != -1.0f) {
			if (m_length == 0.0f) {
				m_length = m_controller->VideoCtrl(EVLC_GET_LENGTH);
			}
			
			if (m_pos.GetRangeMax() <= 1) {
				m_pos.SetRange(0, int(m_length));
			}
			CString strPos;
			strPos.Format(_T("%f/%f"), pos * m_length, m_length);
			SetDlgItemText(IDC_STATIC_TIME, strPos);
			m_pos.SetPos(int(m_length * pos));
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CVideoClientDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	KillTimer(0);
	// TODO: 在此处添加消息处理程序代码
}


void CVideoClientDlg::OnBnClickedBtnPlay()
{
	if (m_status == false) {
		CString url;
		m_url.GetWindowText(url);

		m_controller->SetMedia(m_controller->Unicode2Utf8((LPCTSTR)url));
		m_btnPlay.SetWindowText(_T("暂停"));
		m_status = true;
		m_controller->VideoCtrl(EVLC_PLAY);
	}
	else {
		m_btnPlay.SetWindowText(_T("播放"));
		m_status = false;
		m_controller->VideoCtrl(EVLC_PAUSE);
	}
}


void CVideoClientDlg::OnBnClickedBtnStop()
{
	m_btnPlay.SetWindowText(_T("播放"));
	m_status = false;
	m_controller->VideoCtrl(EVLC_STOP);
}


void CVideoClientDlg::OnTRBNThumbPosChangingSliderPos(NMHDR* pNMHDR, LRESULT* pResult)
{
	// 此功能要求 Windows Vista 或更高版本。
	// _WIN32_WINNT 符号必须 >= 0x0600。
	NMTRBTHUMBPOSCHANGING* pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	TRACE("pos %d reasion %d\r\n", pNMTPC->dwPos, pNMTPC->nReason);
	*pResult = 0;
}


void CVideoClientDlg::OnTRBNThumbPosChangingSliderVolume(NMHDR* pNMHDR, LRESULT* pResult)
{
	// 此功能要求 Windows Vista 或更高版本。
	// _WIN32_WINNT 符号必须 >= 0x0600。
	NMTRBTHUMBPOSCHANGING* pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	TRACE("pos %d reasion %d\r\n", pNMTPC->dwPos, pNMTPC->nReason);
	*pResult = 0;
}

//进度
void CVideoClientDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//TRACE("pos %p volume %p cur %p pos %d\r\n", &m_pos, &m_volume, pScrollBar, nPos);
	if (nSBCode == 5) {
		CString strPosition;
		strPosition.Format(_T("%d%%"), nPos);
		SetDlgItemText(IDC_STATIC_TIME, strPosition);
		m_controller->SetPosition(float(nPos) / m_length);
	}
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

//音量
void CVideoClientDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nSBCode == 5) {
		//TRACE("pos %p volume %p cur %p pos %d\r\n", &m_pos, &m_volume, pScrollBar, nPos);
		CString strVolume;
		strVolume.Format(_T("%d%%"), 100 - nPos);
		SetDlgItemText(IDC_STATIC_VOLUME, strVolume);
		m_controller->SetVolume(100 - nPos);
	}

	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}
