#pragma once
#include "afxwin.h"


// CAr2DCameraDlg dialog

class CAr2DCameraDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAr2DCameraDlg)

public:
	CAr2DCameraDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAr2DCameraDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AR2D_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

// Fields
protected:
	// Main camera
	char m_szMainCameraWin[100];// OpenCV Window Name
	cv::Mat m_mainCameraFrame;
	cv::VideoCapture m_mainCameraReader;
	cv::Size m_sizeMainCameraFrame;

	// Video camera
	char m_szVideoCameraWin[100];// OpenCV Window Name
	cv::Mat m_videoCameraFrame;
	cv::VideoCapture m_videoCameraReader;
	cv::Size m_sizeVideoCameraFrame;

	// Picture camera
	char m_szPictureCameraWin[100];// OpenCV Window Name
	cv::Mat m_pictureCameraFrame;

	bool m_isMainCameraPlaying; // Thread Playing
	bool m_isMainCameraStopping; // Thread Stopping
	CWinThread* m_pMainCameraThread = NULL;

	bool m_isVideoPlaying; // Thread Playing
	bool m_isVideoStopping; // Thread Stopping
	CWinThread* m_pVideoThread = NULL;

	bool m_hasArgument;

	CStatic m_txtLog;
	CString m_log;
protected:
	static DWORD MainCameraThread(LPVOID *lpvParam);
	void MainCameraProcess();
	void StartMainCamera();
	void StopMainCamera();

	static DWORD VideoThread(LPVOID *lpvParam);
	void VideoThreadProcess();
	void StartVideoThread();
	void StopVideoThread();

	void SetLog(CString msg);	
public:
	afx_msg void OnClickedBtnAr2dStart();
	afx_msg void OnClickedBtnAr2DStop();
	afx_msg void OnClickedBtnAr2DPosition();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnClickedBtnAr2DPlay();
};
