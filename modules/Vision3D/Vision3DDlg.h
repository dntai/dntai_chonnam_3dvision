
// Vision3DDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CVision3DDlg dialog
class CVision3DDlg : public CDialogEx
{
// Construction
public:
	CVision3DDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VISION3D_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

// Fields
protected:
	// Main camera
	char m_szMainCameraWin[100];// OpenCV Window Name
	bool m_isMainCameraPlaying; // Thread Playing
	bool m_isMainCameraStopping; // Thread Stopping
	CWinThread* m_pMainCameraThread = NULL;
	cv::VideoCapture m_mainCameraReader;
	cv::Mat m_mainCameraFrame;
	cv::Size m_sizeMainCameraFrame;


	CStatic m_txtLog;
	CString m_log;
	
public:
	CButton m_btnOK;
	afx_msg void OnBnClickedOk();

protected:
	static DWORD MainCameraThread(LPVOID *lpvParam);
	void MainCameraProcess();
	void SetLog(CString msg);

public:
	afx_msg void OnClickedBtnStart();
	void StartMainCamera();
	void StopMainCamera();
	afx_msg void OnClickedBtnStop();
	afx_msg void OnClose();
	afx_msg void OnClickedBtnCalibrate();
	afx_msg void OnClickedBtnAr2DCamera();
};
