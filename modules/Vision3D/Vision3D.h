
// Vision3D.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CVision3DApp:
// See Vision3D.cpp for the implementation of this class
//

class CVision3DApp : public CWinApp
{
public:
	CVision3DApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CVision3DApp theApp;