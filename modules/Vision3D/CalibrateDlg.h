#pragma once
#include "afxwin.h"


// CCalibrateDlg dialog

class CCalibrateDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCalibrateDlg)

public:
	CCalibrateDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCalibrateDlg();

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

	bool m_isMainCameraPlaying; // Thread Playing
	bool m_isMainCameraStopping; // Thread Stopping
	CWinThread* m_pMainCameraThread = NULL;
	
	bool m_isCalibratePlaying; // Thread Playing
	bool m_isCalibrateStopping; // Thread Stopping
	CWinThread* m_pCalibrateThread = NULL;

	bool m_isDetectChessBoardPlaying; // Thread Playing
	bool m_isDetectChessBoardStopping; // Thread Stopping
	CWinThread* m_pDetectChessBoardThread = NULL;

	CStatic m_txtLog;
	CString m_log;

protected:
	static DWORD MainCameraThread(LPVOID *lpvParam);
	void MainCameraProcess();
	void StartMainCamera();
	void StopMainCamera();

	static DWORD CalibrateThread(LPVOID *lpvParam);
	void CalibrateThreadProcess();
	void StartCalibrateThread();
	void StopCalibrateThread();

	static DWORD DetectChessBoardThread(LPVOID *lpvParam);
	void DetectChessBoardThreadProcess();
	void StartDetectChessBoardThread();
	void StopDetectChessBoardThread();

	void SetLog(CString msg);
	afx_msg void OnClose();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnClickedBtnCalib();
	afx_msg void OnClickedBtnChessboard();
	afx_msg void OnClickedBtnLoadCalib();
	CStatic m_txtCalibInfo;
};
