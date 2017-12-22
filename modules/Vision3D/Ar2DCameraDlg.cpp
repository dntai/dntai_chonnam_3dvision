// Ar2DCameraDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Vision3D.h"
#include "Ar2DCameraDlg.h"
#include "afxdialogex.h"


// CAr2DCameraDlg dialog

IMPLEMENT_DYNAMIC(CAr2DCameraDlg, CDialogEx)

CAr2DCameraDlg::CAr2DCameraDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_AR2D_DIALOG, pParent)
{
	std::sprintf(this->m_szMainCameraWin, "Calibrate Camera");
	std::sprintf(this->m_szVideoCameraWin, "Video Camera");
	std::sprintf(this->m_szPictureCameraWin, "Picture Camera");
	this->m_isMainCameraPlaying = false;
	this->m_isVideoPlaying = false;
}

CAr2DCameraDlg::~CAr2DCameraDlg()
{
}

void CAr2DCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_LOG, m_txtLog);
}


BEGIN_MESSAGE_MAP(CAr2DCameraDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_AR2D_START, &CAr2DCameraDlg::OnClickedBtnAr2dStart)
	ON_BN_CLICKED(IDC_BUTTON_AR2D_STOP, &CAr2DCameraDlg::OnClickedBtnAr2DStop)
	ON_BN_CLICKED(IDC_BUTTON_AR2D_POSITION, &CAr2DCameraDlg::OnClickedBtnAr2DPosition)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_AR2D_PLAY, &CAr2DCameraDlg::OnClickedBtnAr2DPlay)
END_MESSAGE_MAP()


// CAr2DCameraDlg message handlers



//////////////////////////////////////////////////////////
DWORD CAr2DCameraDlg::MainCameraThread(LPVOID *lpvParam)
{
	// [TODO]
	CAr2DCameraDlg *pThread = (CAr2DCameraDlg*)lpvParam;
	pThread->MainCameraProcess();
	return 0;
}
void CAr2DCameraDlg::MainCameraProcess()
{
	// [TODO]
	int cnt, key;
	
	cv::Mat frameCopy;

	this->m_isMainCameraPlaying = true;
	this->m_isMainCameraStopping = false;
	
	bool showRejected = false;
	float markerLength = 0.1;

	cv::Ptr<cv::aruco::DetectorParameters> detectorParams = cv::aruco::DetectorParameters::create();
	detectorParams->cornerRefinementMethod = cv::aruco::CORNER_REFINE_SUBPIX; // do corner refinement in markers
	
	int dictionaryId = cv::aruco::DICT_6X6_100;
	cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));
	
	cv::Mat camMatrix, distCoeffs;
	bool estimatePose = true;
	if (estimatePose)
	{
		cv::FileStorage fs("Data\\aruco_calib.xml", cv::FileStorage::READ);
		fs["Intrinsic"] >> camMatrix;
		fs["Distortion"] >> distCoeffs;
	}

	cv::Mat ARFrame, ARDestFrame, blankFrame, maskFrame, image;
	cv::Mat warp_matrix;
	std::vector<cv::Point2f> sourcePoints, destPoints;
	int srcWidth, srcHeight;
	
	for (cnt = 1, key = -1; this->m_mainCameraReader.read(this->m_mainCameraFrame) == true && key != 27 && this->m_isMainCameraStopping == false; cnt++)
	{
		m_log.Format(_T("Read Frame #%d"), cnt); this->SetLog(this->m_log);
		
		std::vector< int > ids;
		std::vector< std::vector< cv::Point2f > > corners, rejected;
		std::vector < cv::Vec3d > rvecs, tvecs;

		// detect markers and estimate pose
		cv::aruco::detectMarkers(this->m_mainCameraFrame, dictionary, corners, ids, detectorParams, rejected);
		if (estimatePose && ids.size() > 0)
		{
			cv::aruco::estimatePoseSingleMarkers(corners, markerLength, camMatrix, distCoeffs, rvecs, tvecs);
		}

		// draw results
		this->m_mainCameraFrame.copyTo(frameCopy);
		if (ids.size() > 0)
		{
			cv::aruco::drawDetectedMarkers(this->m_mainCameraFrame, corners, ids);

			if (estimatePose)
			{
				for (unsigned int i = 0; i < ids.size(); i++)
					cv::aruco::drawAxis(this->m_mainCameraFrame, camMatrix, distCoeffs, rvecs[i], tvecs[i], markerLength * 0.5f);
			}
		}

		if (showRejected && rejected.size() > 0)
		{
			cv::aruco::drawDetectedMarkers(this->m_mainCameraFrame, rejected, cv::noArray(), cv::Scalar(100, 0, 255));
		}

		if (this->m_hasArgument && ids.size() > 0)
		{
			blankFrame = cv::Mat::zeros(this->m_mainCameraFrame.size(), this->m_mainCameraFrame.type());
			cv::bitwise_not(blankFrame, blankFrame);
			srcWidth = this->m_mainCameraFrame.size().width;
			srcHeight = this->m_mainCameraFrame.size().height;

			sourcePoints.clear();
			sourcePoints.push_back(cv::Point2f(0, 0));
			sourcePoints.push_back(cv::Point2f(srcWidth, 0));
			sourcePoints.push_back(cv::Point2f(srcWidth, srcHeight));
			sourcePoints.push_back(cv::Point2f(0, srcHeight));

			frameCopy.copyTo(image);
			for (int i = 0; i < ids.size(); i++)
			{
				if (ids[i] == 3 && this->m_pictureCameraFrame.empty() == false)
				{
					this->m_pictureCameraFrame.copyTo(ARFrame);

					destPoints.clear();
					for (int j = 0; j < sourcePoints.size(); j++)
					{
						destPoints.push_back(corners[i][j]);
					}
					warp_matrix = cv::getPerspectiveTransform(sourcePoints, destPoints);

					cv::warpPerspective(ARFrame, ARDestFrame, warp_matrix, image.size());


					cv::warpPerspective(blankFrame, maskFrame, warp_matrix, image.size());
					cv::bitwise_not(maskFrame, maskFrame);
					cv::bitwise_and(maskFrame, image, maskFrame);

					cv::bitwise_or(ARDestFrame, maskFrame, image);
				}
				if (ids[i] == 1 && this->m_videoCameraFrame.empty() == false)
				{
					this->m_videoCameraFrame.copyTo(ARFrame);

					destPoints.clear();
					for (int j = 0; j < sourcePoints.size(); j++)
					{
						destPoints.push_back(corners[i][j]);
					}
					warp_matrix = cv::getPerspectiveTransform(sourcePoints, destPoints);

					cv::warpPerspective(ARFrame, ARDestFrame, warp_matrix, image.size());


					cv::warpPerspective(blankFrame, maskFrame, warp_matrix, image.size());
					cv::bitwise_not(maskFrame, maskFrame);
					cv::bitwise_and(maskFrame, image, maskFrame);

					cv::bitwise_or(ARDestFrame, maskFrame, image);
				}
			}
			image.copyTo(this->m_mainCameraFrame);
		}
		
		cv::imshow(this->m_szMainCameraWin, this->m_mainCameraFrame);
		key = cv::waitKey(1);
	}

	this->m_isMainCameraPlaying = false;
}
void CAr2DCameraDlg::StartMainCamera()
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
		m_pMainCameraThread = AfxBeginThread((AFX_THREADPROC)CAr2DCameraDlg::MainCameraThread, (LPVOID) this);
	}
	else
	{
		this->SetLog(L"Camera is already started!");
	}
}
void CAr2DCameraDlg::StopMainCamera()
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
		// this->m_mainCameraFrame.release();
	}
}

DWORD CAr2DCameraDlg::VideoThread(LPVOID *lpvParam)
{
	// [TODO]
	CAr2DCameraDlg *pThread = (CAr2DCameraDlg*)lpvParam;
	pThread->VideoThreadProcess();
	return 0;
}
void CAr2DCameraDlg::VideoThreadProcess()
{
	// [TODO]
	int cnt, key;

	this->m_isVideoPlaying = true;
	this->m_isVideoStopping = false;
	for (cnt = 1, key = -1; this->m_videoCameraReader.read(this->m_videoCameraFrame) == true && key != 27 && this->m_isVideoStopping == false; cnt++)
	{
		m_log.Format(_T("Read Frame #%d"), cnt); this->SetLog(this->m_log);
		cv::imshow(this->m_szVideoCameraWin, this->m_videoCameraFrame);
		key = cv::waitKey(1);
	}

	this->m_isVideoPlaying = false;
}
void CAr2DCameraDlg::StartVideoThread()
{
	int cnt;

	if (m_isVideoPlaying == false)
	{
		// Open Camera
		this->m_videoCameraReader.open("Data//Demo.mp4");
		if (this->m_videoCameraReader.isOpened() == false)
		{
			this->SetLog(L"Video is not opened!");
			return;
		}

		// Read information camera
		if (this->m_videoCameraReader.read(this->m_videoCameraFrame) == false)
		{
			this->SetLog(_T("Read video information failed!"));
			return;
		}

		this->m_sizeVideoCameraFrame = this->m_videoCameraFrame.size();

		// Process Camera
		m_pVideoThread = AfxBeginThread((AFX_THREADPROC)CAr2DCameraDlg::VideoThread, (LPVOID) this);
	}
	else
	{
		this->SetLog(L"Video is already started!");
	}
}
void CAr2DCameraDlg::StopVideoThread()
{
	DWORD exit_code = NULL;
	if (this->m_pVideoThread != NULL)
	{
		this->m_isVideoStopping = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (this->m_isVideoPlaying == true)
		{
			GetExitCodeThread(this->m_pVideoThread->m_hThread, &exit_code);
			if (exit_code == STILL_ACTIVE)
			{
				::TerminateThread(this->m_pVideoThread->m_hThread, 0);
				CloseHandle(this->m_pVideoThread->m_hThread);
			}
		}

		// Release
		this->m_pVideoThread->m_hThread = NULL;
		this->m_pVideoThread = NULL;
		this->m_isVideoPlaying = false;

		this->m_videoCameraReader.release();
		this->m_videoCameraFrame.release();
	}
}

void CAr2DCameraDlg::SetLog(CString msg)
{
	m_txtLog.SetWindowTextW(CString("Log: ") + msg);
}

void CAr2DCameraDlg::OnClickedBtnAr2dStart()
{
	// TODO: Add your control notification handler code here
	this->StartMainCamera();
}


void CAr2DCameraDlg::OnClickedBtnAr2DStop()
{
	// TODO: Add your control notification handler code here
	this->StopMainCamera();
}


void CAr2DCameraDlg::OnClickedBtnAr2DPosition()
{
	// TODO: Add your control notification handler code here
}


void CAr2DCameraDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	this->StopMainCamera();
	this->StopVideoThread();

	CDialogEx::OnClose();
}


BOOL CAr2DCameraDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	if (this->m_isMainCameraPlaying == false)
	{
		cv::namedWindow(this->m_szMainCameraWin, CV_WINDOW_NORMAL);
		cv::resizeWindow(this->m_szMainCameraWin, 598, 526);
		HWND hWnd = (HWND)cvGetWindowHandle(this->m_szMainCameraWin);
		HWND hParent = ::GetParent(hWnd);
		::SetParent(hWnd, GetDlgItem(IDC_MAIN_CAMERA)->m_hWnd);
		::ShowWindow(hParent, SW_HIDE);
	}

	if (this->m_isVideoPlaying == false)
	{
		cv::namedWindow(this->m_szVideoCameraWin, CV_WINDOW_NORMAL);
		cv::resizeWindow(this->m_szVideoCameraWin, 336, 260);
		HWND hWnd = (HWND)cvGetWindowHandle(this->m_szVideoCameraWin);
		HWND hParent = ::GetParent(hWnd);
		::SetParent(hWnd, GetDlgItem(IDC_VIDEO_CAMERA)->m_hWnd);
		::ShowWindow(hParent, SW_HIDE);
	}

	if (this->m_isMainCameraPlaying == false)
	{
		cv::namedWindow(this->m_szPictureCameraWin, CV_WINDOW_NORMAL);
		cv::resizeWindow(this->m_szPictureCameraWin, 336, 260);
		HWND hWnd = (HWND)cvGetWindowHandle(this->m_szPictureCameraWin);
		HWND hParent = ::GetParent(hWnd);
		::SetParent(hWnd, GetDlgItem(IDC_PICTURE_CAMERA)->m_hWnd);
		::ShowWindow(hParent, SW_HIDE);
	}
	
	this->m_hasArgument = false;
	GetDlgItem(IDC_BUTTON_AR2D_PLAY)->SetWindowTextW(L"Ar&gument");

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CAr2DCameraDlg::OnClickedBtnAr2DPlay()
{
	// TODO: Add your control notification handler code here
	if (m_hasArgument == false)
	{
		GetDlgItem(IDC_BUTTON_AR2D_PLAY)->SetWindowTextW(L"Unar&gument");
		this->StartVideoThread();
		this->m_pictureCameraFrame = cv::imread("Data//lena_std.tif");
		cv::imshow(this->m_szPictureCameraWin, this->m_pictureCameraFrame);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_AR2D_PLAY)->SetWindowTextW(L"Ar&gument");
		this->StopVideoThread();
	}
	m_hasArgument = !m_hasArgument;
}
