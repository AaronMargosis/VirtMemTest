
// VirtMemTest.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "VirtMemTest.h"
#include "VirtMemTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/// <summary>
/// Global flag indicating whether the app should deliberately crash on exit
/// </summary>
BOOL g_bCrash = FALSE;

// CVirtMemTestApp

BEGIN_MESSAGE_MAP(CVirtMemTestApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CVirtMemTestApp construction

CVirtMemTestApp::CVirtMemTestApp()
{
}


// The one and only CVirtMemTestApp object

CVirtMemTestApp theApp;


// CVirtMemTestApp initialization

BOOL CVirtMemTestApp::InitInstance()
{
	CWinAppEx::InitInstance();

	// Display the dialog modally.
	CVirtMemTestDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();
	// If the global g_bCrash flag has been set, perform an illegal action to crash the app.
	if (g_bCrash)
	{
		// Null-pointer write should crash the app
		DWORD* pBadAddr = nullptr;
		*pBadAddr = 0x12341234;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
