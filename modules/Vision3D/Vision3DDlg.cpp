
// Vision3DDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Vision3D.h"
#include "Vision3DDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CVision3DDlg dialog
CVision3DDlg::CVision3DDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_VISION3D_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// [TODO] Construction method
	std::sprintf(this->m_szMainCameraWin, "Main Camera");
	this->m_isMainCameraPlaying = false;
}

void CVision3DDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_STATIC_LOG, m_txtLog);
}

BEGIN_MESSAGE_MAP(CVision3DDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CVision3DDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_START, &CVision3DDlg::OnClickedBtnStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CVision3DDlg::OnClickedBtnStop)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_CALIBRATE, &CVision3DDlg::OnClickedBtnCalibrate)
	ON_BN_CLICKED(IDC_BUTTON_AR2DCAMERA, &CVision3DDlg::OnClickedBtnAr2DCamera)
END_MESSAGE_MAP()


// CVision3DDlg message handlers
BOOL CVision3DDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	
	// [TODO] Initialization User Graphic Interface
	if (this->m_isMainCameraPlaying == false)
	{
		cv::namedWindow(this->m_szMainCameraWin, CV_WINDOW_NORMAL);
		cv::resizeWindow(this->m_szMainCameraWin, 480, 270);
		HWND hWnd = (HWND)cvGetWindowHandle(this->m_szMainCameraWin);
		HWND hParent = ::GetParent(hWnd);
		::SetParent(hWnd, GetDlgItem(IDC_MAIN_CAMERA)->m_hWnd);
		::ShowWindow(hParent, SW_HIDE);
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVision3DDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVision3DDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVision3DDlg::OnBnClickedOk()
{
	// [TODO]
	cv::String fname;
	cv::Mat img;

	fname = "Data//lena_std.tif";
	img = imread(fname, cv::IMREAD_COLOR);
	cv::imshow(this->m_szMainCameraWin, img);

	// CDialogEx::OnOK();
}


DWORD CVision3DDlg::MainCameraThread(LPVOID *lpvParam)
{
	CVision3DDlg *pThread = (CVision3DDlg*)lpvParam;
	pThread->MainCameraProcess();
	return 0;
}


void CVision3DDlg::MainCameraProcess()
{
	// [TODO]
	int cnt, key;

	this->m_isMainCameraPlaying = true;
	this->m_isMainCameraStopping = false;
	for (cnt = 1, key = -1; this->m_mainCameraReader.read(this->m_mainCameraFrame) == true && key != 27 && this->m_isMainCameraStopping == false; cnt++)
	{
		m_log.Format(_T("Read Frame #%d"), cnt); this->SetLog(this->m_log);
		cv::imshow(this->m_szMainCameraWin, this->m_mainCameraFrame);
		
		
		key = cv::waitKey(1);
	}

	m_isMainCameraPlaying = false;
}

void CVision3DDlg::StartMainCamera()
{
	int cnt;

	if (m_isMainCameraPlaying == false)
	{
		// Open Camera
		this->m_mainCameraReader.open(0);
		if (this->m_mainCameraReader.isOpened() == false)
		{
			this->SetLog(L"Camera is not opened!");
			return;
		}

		// Read information camera
		for (cnt = 1; !this->m_mainCameraReader.retrieve(this->m_mainCameraFrame) && cnt <= 10; cnt++)
		{
			m_log.Format(_T("Retry read camera information %d time(s)!"), cnt); this->SetLog(m_log);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		if (cnt > 10)
		{
			this->SetLog(_T("Read camera information failed!"));
			return;
		}

		this->m_sizeMainCameraFrame = this->m_mainCameraFrame.size();

		// Process Camera
		m_pMainCameraThread = AfxBeginThread((AFX_THREADPROC)CVision3DDlg::MainCameraThread, (LPVOID) this);
	}
	else
	{
		AfxMessageBox(L"Camera is already started!");
	}
}

void CVision3DDlg::StopMainCamera()
{
	DWORD exit_code = NULL;
	if (this->m_pMainCameraThread != NULL)
	{
		this->m_isMainCameraStopping = true;
		if (this->m_pMainCameraThread != NULL)
			WaitForSingleObject(this->m_pMainCameraThread->m_hThread, 100);
		// std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (this->m_isMainCameraPlaying == true)
		{
			GetExitCodeThread(this->m_pMainCameraThread->m_hThread, &exit_code);
			if (exit_code == STILL_ACTIVE)
			{
				::TerminateThread(this->m_pMainCameraThread->m_hThread, 0);
				CloseHandle(this->m_pMainCameraThread->m_hThread);
			}
		}
		
		// Release
		this->m_pMainCameraThread->m_hThread = NULL;
		this->m_pMainCameraThread = NULL;
		this->m_isMainCameraPlaying = false;

		this->m_mainCameraReader.release();
		this->m_mainCameraFrame.release();
	}
}

void CVision3DDlg::OnClickedBtnStart()
{
	// TODO: Add your control notification handler code here
	this->StartMainCamera();
}

void CVision3DDlg::SetLog(CString msg)
{
	m_txtLog.SetWindowTextW(CString("Log: ") + msg);
}



void CVision3DDlg::OnClickedBtnStop()
{
	// TODO: Add your control notification handler code here
	this->StopMainCamera();
}


void CVision3DDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	this->StopMainCamera();

	CDialogEx::OnClose();
}


void CVision3DDlg::OnClickedBtnCalibrate()
{
	// TODO: Add your control notification handler code here
	CCalibrateDlg dlg = new CCalibrateDlg();
	dlg.DoModal();
}


void CVision3DDlg::OnClickedBtnAr2DCamera()
{
	// TODO: Add your control notification handler code here
	CAr2DCameraDlg dlg = new CAr2DCameraDlg();
	dlg.DoModal();
}
