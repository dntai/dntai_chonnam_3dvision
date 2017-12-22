// CalibrateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Vision3D.h"
#include "CalibrateDlg.h"
#include "afxdialogex.h"


// CCalibrateDlg dialog

IMPLEMENT_DYNAMIC(CCalibrateDlg, CDialogEx)

CCalibrateDlg::CCalibrateDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CALIBRATION_DIALOG, pParent)
{
	// [TODO] Construction method
	Utilities::mkdir("Data//boards");

	std::sprintf(this->m_szMainCameraWin, "Calibrate Camera");
	this->m_isMainCameraPlaying = false;
	this->m_isCalibratePlaying = false;
	this->m_isDetectChessBoardPlaying = false;
}

CCalibrateDlg::~CCalibrateDlg()
{
}

void CCalibrateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_LOG, m_txtLog);
	DDX_Control(pDX, IDC_STATIC_CALIB_INFO, m_txtCalibInfo);
}


BEGIN_MESSAGE_MAP(CCalibrateDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CCalibrateDlg::OnBnClickedOk)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_CALIB, &CCalibrateDlg::OnClickedBtnCalib)
	ON_BN_CLICKED(IDC_BUTTON_CHESSBOARD, &CCalibrateDlg::OnClickedBtnChessboard)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_CALIB, &CCalibrateDlg::OnClickedBtnLoadCalib)
END_MESSAGE_MAP()


// CCalibrateDlg message handlers
DWORD CCalibrateDlg::MainCameraThread(LPVOID *lpvParam)
{
	CCalibrateDlg *pThread = (CCalibrateDlg*)lpvParam;
	pThread->MainCameraProcess();
	return 0;
}

void CCalibrateDlg::MainCameraProcess()
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

	this->m_isMainCameraPlaying = false;
}

void CCalibrateDlg::StartMainCamera()
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
		m_pMainCameraThread = AfxBeginThread((AFX_THREADPROC)CCalibrateDlg::MainCameraThread, (LPVOID) this);
	}
	else
	{
		AfxMessageBox(L"Camera is already started!");
	}
}

void CCalibrateDlg::StopMainCamera()
{
	DWORD exit_code = NULL;
	if (this->m_pMainCameraThread != NULL)
	{
		this->m_isMainCameraStopping = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

void CCalibrateDlg::SetLog(CString msg)
{
	m_txtLog.SetWindowTextW(CString("Log: ") + msg);
}

void CCalibrateDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	this->StopMainCamera();
	this->StopCalibrateThread();
	this->StopDetectChessBoardThread();

	CDialogEx::OnClose();
}

BOOL CCalibrateDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	// [TODO] Initialization User Graphic Interface
	if (this->m_isMainCameraPlaying == false)
	{
		cv::namedWindow(this->m_szMainCameraWin, CV_WINDOW_NORMAL);
		cv::resizeWindow(this->m_szMainCameraWin, 960, 540);
		HWND hWnd = (HWND)cvGetWindowHandle(this->m_szMainCameraWin);
		HWND hParent = ::GetParent(hWnd);
		::SetParent(hWnd, GetDlgItem(IDC_MAIN_CAMERA)->m_hWnd);
		::ShowWindow(hParent, SW_HIDE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CCalibrateDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	this->StartMainCamera();

	// CDialogEx::OnOK();
}




void CCalibrateDlg::OnClickedBtnCalib()
{
	// TODO: Add your control notification handler code here
	this->StartCalibrateThread();
}


void CCalibrateDlg::OnClickedBtnChessboard()
{
	// TODO: Add your control notification handler code here
	this->StartDetectChessBoardThread();
}


DWORD CCalibrateDlg::CalibrateThread(LPVOID *lpvParam)
{
	CCalibrateDlg *pThread = (CCalibrateDlg*)lpvParam;
	pThread->CalibrateThreadProcess();
	return 0;
}

void CCalibrateDlg::CalibrateThreadProcess()
{
	////////////////////////////////////////////
	this->m_isCalibratePlaying = true;
	this->m_isMainCameraStopping = false;
	m_log.Format(_T("Calibration started!")); this->SetLog(m_log);
	////////////////////////////////////////////

	cv::Mat image;
	char name[255];

	int success;
	CameraCalibrator cameraCalibrator;
	std::vector<cv::Point2f> imageCorners;

	for (int i = 1; i <= 10; i++)
	{
		sprintf(name, "DATA\\boards\\board%d.png", i);
		image = cv::imread(name);

		success = cameraCalibrator.addChessboardImage(image, cv::Size(6, 4), imageCorners);
		cv::drawChessboardCorners(image, cv::Size(6, 4), imageCorners, success > 0);
		cv::imshow(this->m_szMainCameraWin, image);
		cv::waitKey(100);
	}

	// calibrate the camera
	cameraCalibrator.setCalibrationFlag(true, true);
	cameraCalibrator.calibrate(image.size());

	cameraCalibrator.showCameraParameters();

	// Image Undistortion
	sprintf(name, "DATA\\boards\\board%d.png", 6);
	image = cv::imread(name);
	cv::Mat uImage = cameraCalibrator.undistort(image);

	cv::namedWindow("Original Image");
	cv::namedWindow("Undistorted Image");
	cv::imshow("Original Image", image);
	cv::imshow("Undistorted Image", uImage);

	// Store everything in a xml file
	cameraCalibrator.saveCameraParameters("DATA//calib.xml");
	this->m_txtCalibInfo.SetWindowTextW((LPCTSTR)cameraCalibrator.toStringCameraParameters().c_str());

	////////////////////////////////////////////
	m_log.Format(_T("Calibration stopped!")); this->SetLog(m_log);
	this->m_isCalibratePlaying = false;
	////////////////////////////////////////////

	cv::waitKey(0);
	cv::destroyWindow("Original Image");
	cv::destroyWindow("Undistorted Image");
}

void CCalibrateDlg::StartCalibrateThread()
{
	int cnt;

	if (m_isCalibratePlaying == false)
	{
		// Process Camera
		m_pCalibrateThread = AfxBeginThread((AFX_THREADPROC)CCalibrateDlg::CalibrateThread, (LPVOID) this);
	}
	else
	{
		AfxMessageBox(L"Calibrate is already started!");
	}
}
void CCalibrateDlg::StopCalibrateThread()
{
	DWORD exit_code = NULL;
	if (this->m_pCalibrateThread != NULL)
	{
		this->m_isCalibrateStopping = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (this->m_isCalibratePlaying == true)
		{
			GetExitCodeThread(this->m_pCalibrateThread->m_hThread, &exit_code);
			if (exit_code == STILL_ACTIVE)
			{
				::TerminateThread(this->m_pCalibrateThread->m_hThread, 0);
				CloseHandle(this->m_pCalibrateThread->m_hThread);
			}
		}

		// Release
		this->m_pCalibrateThread->m_hThread = NULL;
		this->m_pCalibrateThread = NULL;
		this->m_isCalibratePlaying = false;
	}
}

///////////////////////////////
DWORD CCalibrateDlg::DetectChessBoardThread(LPVOID *lpvParam)
{
	CCalibrateDlg *pThread = (CCalibrateDlg*)lpvParam;
	pThread->DetectChessBoardThreadProcess();
	return 0;
}

void CCalibrateDlg::DetectChessBoardThreadProcess()
{
	////////////////////////////////////////////
	this->m_isDetectChessBoardPlaying = true;
	this->m_isDetectChessBoardStopping = false;
	m_log.Format(_T("Detect ChessBoard started!")); this->SetLog(m_log);
	////////////////////////////////////////////

	int key, cnt;
	cv::Mat frame, gray, gray1, frame1, tmp;
	int width, height;


	int success;
	CameraCalibrator cameraCalibrator;
	std::vector<cv::Point2f> imageCorners;

	// Process camera
	int nsave = 1;
	char name[255];
	for (cnt = 1, key = -1; this->m_mainCameraReader.read(frame) == true && key != 27; cnt++)
	{
		m_log.Format(_T("Read Frame #%d"), cnt); this->SetLog(m_log);
		
		success = cameraCalibrator.findChessboardImage(frame, cv::Size(6, 4), imageCorners);

		if (success)
		{
			frame.copyTo(tmp);
			cv::drawChessboardCorners(tmp, cv::Size(6, 4), imageCorners, true);
			cv::imshow("ChessBoard Result", tmp);
			m_log.Format(_T("Chessboard Found! Press Space to save, others to continue...")); this->SetLog(m_log);
			key = cv::waitKey(0);
			cv::destroyWindow("ChessBoard Result");
			if (key == 32)
			{
				sprintf(name, "DATA\\boards\\board%d.png", nsave);
				cv::imwrite(name, frame);
				nsave++;
			}
		}

		cv::imshow(this->m_szMainCameraWin, frame);
		cv::waitKey(1);
	}

	////////////////////////////////////////////
	m_log.Format(_T("Detect ChessBoard stopped!")); this->SetLog(m_log);
	this->m_isDetectChessBoardPlaying = false;
	////////////////////////////////////////////

	cv::destroyWindow("ChessBoard Result");
}
void CCalibrateDlg::StartDetectChessBoardThread()
{
	int cnt;

	if (m_isDetectChessBoardPlaying == false)
	{
		this->m_mainCameraReader.open(0);

		// Check camera open
		if (!this->m_mainCameraReader.isOpened())
		{
			m_log.Format(_T("Load camera failed!")); this->SetLog(m_log);
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
			m_log.Format(_T("Read camera information failed!")); this->SetLog(m_log);
			return;
		}

		this->m_sizeMainCameraFrame = this->m_mainCameraFrame.size();

		// Process Camera
		m_pDetectChessBoardThread = AfxBeginThread((AFX_THREADPROC)CCalibrateDlg::DetectChessBoardThread, (LPVOID) this);
	}
	else
	{
		AfxMessageBox(L"Detect ChessBoard is already started!");
	}
}
void CCalibrateDlg::StopDetectChessBoardThread()
{
	DWORD exit_code = NULL;
	if (this->m_pDetectChessBoardThread != NULL)
	{
		this->m_isDetectChessBoardStopping = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (this->m_isDetectChessBoardPlaying == true)
		{
			GetExitCodeThread(this->m_pDetectChessBoardThread->m_hThread, &exit_code);
			if (exit_code == STILL_ACTIVE)
			{
				::TerminateThread(this->m_pDetectChessBoardThread->m_hThread, 0);
				CloseHandle(this->m_pDetectChessBoardThread->m_hThread);
			}
		}

		// Release
		this->m_pDetectChessBoardThread->m_hThread = NULL;
		this->m_pDetectChessBoardThread = NULL;
		this->m_isDetectChessBoardPlaying = false;
	}
}

void CCalibrateDlg::OnClickedBtnLoadCalib()
{
	// TODO: Add your control notification handler code here
	CameraCalibrator cameraCalibrator;
	cameraCalibrator.readCameraParameters("DATA\\calib.xml");
	this->m_txtCalibInfo.SetWindowTextW((LPCTSTR)cameraCalibrator.toStringCameraParameters().c_str());
}
