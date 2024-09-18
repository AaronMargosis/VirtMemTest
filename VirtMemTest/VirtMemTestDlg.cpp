
// VirtMemTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include <vector>
#include "VirtMemTest.h"
#include "VirtMemTestDlg.h"
#include "SharedMem.h"
#include "SysErrorMessage.h"
#include "HEX.h"
#include "DbgOut.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVirtMemTestDlg dialog

enum eCpuBoundFileTouch_t { eNoFileTouch, eOneTimeFileTouch, eRepeatedFileTouch };
enum eExceptionType_t { eET_int, eET_float, eET_stdString, eET_stdWstring, eET_voidStar, eET_charStar, eET_wcharStar, eET_customClass, eET_divByZero, eET_breakpoint, eET_memoryAccessViolation };


CVirtMemTestDlg::CVirtMemTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVirtMemTestDlg::IDD, pParent)
	, m_dwMbToAllocate(100)
	, m_bCommit(FALSE)
	, m_bFree(FALSE)
	, m_bWrite(FALSE)
	, m_bHog(FALSE)
	, m_bSingleCPU(FALSE)
	, m_bThreadToCPUAffinity(FALSE)
	, m_dwHangSecs(0)
	, m_bCrashOnEndSession(FALSE)
	, m_bExecute(FALSE)
	, m_bExecuteNOPs(FALSE)
	, m_bRead(FALSE)
	, m_CpuHogsCount(1)
	, m_sDebugText(_T(""))
	, m_bCatchException(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVirtMemTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MbToAllocate, m_dwMbToAllocate);
	DDX_Check(pDX, IDC_Commit, m_bCommit);
	DDX_Check(pDX, IDC_Free, m_bFree);
	DDX_Check(pDX, IDC_Write, m_bWrite);
	DDX_Check(pDX, IDC_CpuHog, m_bHog);
	DDX_Check(pDX, IDC_SingleCPU, m_bSingleCPU);
	DDX_Check(pDX, IDC_ThreadToCPUAffinity, m_bThreadToCPUAffinity);
	DDX_Control(pDX, IDC_AddrList, m_list);
	DDX_Text(pDX, IDC_HangSecs, m_dwHangSecs);
	DDX_Check(pDX, IDC_CrashOnEndSession, m_bCrashOnEndSession);
	DDX_Check(pDX, IDC_Execute, m_bExecute);
	DDX_Check(pDX, IDC_ExecuteNops, m_bExecuteNOPs);
	DDX_Check(pDX, IDC_Read, m_bRead);
	DDX_Control(pDX, IDC_PageProtection, m_cmbPageProtectionCtl);
	DDX_Text(pDX, IDC_CpuHogsCount, m_CpuHogsCount);
	DDV_MinMaxUInt(pDX, m_CpuHogsCount, 1, 99);
	DDX_Control(pDX, IDC_ThreadPriority, m_cmbThreadPriorityCtl);
	DDX_Control(pDX, IDC_CpuHogsCount, m_ctlThreadCount);
	DDX_Control(pDX, IDC_SingleCPU, m_ctlSingleCPU);
	DDX_Control(pDX, IDC_ThreadToCPUAffinity, m_ctlThreadToCPUAffinity);
	DDX_Control(pDX, IDC_CpuBoundFileTouch, m_cmbCpuBoundFileTouch);
	DDX_Text(pDX, IDC_DebugText, m_sDebugText);
	DDX_Check(pDX, IDC_CatchException, m_bCatchException);
	DDX_Control(pDX, IDC_ExceptionType, m_ctlExceptionType);
}

BEGIN_MESSAGE_MAP(CVirtMemTestDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_VirtualAlloc, &CVirtMemTestDlg::OnBnClickedVirtualAlloc)
	ON_BN_CLICKED(IDC_CpuHog, &CVirtMemTestDlg::OnBnClickedCpuhog)
	ON_BN_CLICKED(IDC_CreateFileMapping, &CVirtMemTestDlg::OnBnClickedCreatefilemapping)
	ON_BN_CLICKED(IDC_Stack, &CVirtMemTestDlg::OnBnClickedStack)
	ON_BN_CLICKED(IDC_SingleCPU, &CVirtMemTestDlg::OnBnClickedSinglecpu)
	ON_BN_CLICKED(IDC_ThreadToCPUAffinity, &CVirtMemTestDlg::OnBnClickedThreadToCPUAffinity)
	ON_BN_CLICKED(IDC_Hang, &CVirtMemTestDlg::OnBnClickedHang)
	ON_WM_ENDSESSION()
	ON_WM_CLOSE()
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(IDC_CrashOnEndSession, &CVirtMemTestDlg::OnBnClickedCrashonendsession)
	ON_BN_CLICKED(IDC_newbyte, &CVirtMemTestDlg::OnBnClickednewbyte)
	ON_BN_CLICKED(IDC_OutputDebugString, &CVirtMemTestDlg::OnBnClickedOutputdebugstring)
	//ON_BN_CLICKED(IDC_ThrowBaseType, &CVirtMemTestDlg::OnBnClickedThrowbasetype)
	//ON_BN_CLICKED(IDC_ThrowCustomClass, &CVirtMemTestDlg::OnBnClickedThrowcustomclass)
	ON_BN_CLICKED(IDC_ThrowException, &CVirtMemTestDlg::OnBnClickedThrowexception)
END_MESSAGE_MAP()


// CVirtMemTestDlg message handlers
static int AddStringAndItemDataToCombo(CComboBox & cmb, DWORD dwItemData, const wchar_t * szString)
{
	int ix = cmb.AddString(szString);
	if ( CB_ERR != ix )
	{
		cmb.SetItemData(ix, dwItemData);
	}
	return ix;
}

BOOL CVirtMemTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CString sTitle;
	this->GetWindowTextW(sTitle);
	sTitle += (sizeof(void*) == 4) ? L" -- 32-BIT" : L" -- 64-BIT";
	this->SetWindowTextW(sTitle);

	int ix;

	// Initialize the page protection combo
	//TODO: make it possible to add PAGE_GUARD, PAGE_NOCACHE, PAGE_WRITECOMBINE; also see the PAGE_TARGETS_* constants
	AddStringAndItemDataToCombo(m_cmbPageProtectionCtl, PAGE_EXECUTE,           L"PAGE_EXECUTE");
	AddStringAndItemDataToCombo(m_cmbPageProtectionCtl, PAGE_EXECUTE_READ,      L"PAGE_EXECUTE_READ");
	AddStringAndItemDataToCombo(m_cmbPageProtectionCtl, PAGE_EXECUTE_READWRITE, L"PAGE_EXECUTE_READWRITE");
	AddStringAndItemDataToCombo(m_cmbPageProtectionCtl, PAGE_EXECUTE_WRITECOPY, L"PAGE_EXECUTE_WRITECOPY");
	AddStringAndItemDataToCombo(m_cmbPageProtectionCtl, PAGE_NOACCESS,          L"PAGE_NOACCESS");
	ix = 
	AddStringAndItemDataToCombo(m_cmbPageProtectionCtl, PAGE_READONLY,          L"PAGE_READONLY");
	AddStringAndItemDataToCombo(m_cmbPageProtectionCtl, PAGE_READWRITE,         L"PAGE_READWRITE");
	AddStringAndItemDataToCombo(m_cmbPageProtectionCtl, PAGE_WRITECOPY,         L"PAGE_WRITECOPY");
	if ( CB_ERR != ix )
	{
		m_cmbPageProtectionCtl.SetCurSel(ix);
	}

	// Initialize the thread priority combo
	AddStringAndItemDataToCombo(m_cmbThreadPriorityCtl, (DWORD)THREAD_PRIORITY_LOWEST, L"LOWEST");
	AddStringAndItemDataToCombo(m_cmbThreadPriorityCtl, (DWORD)THREAD_PRIORITY_BELOW_NORMAL, L"BELOW_NORMAL");
	ix =
	AddStringAndItemDataToCombo(m_cmbThreadPriorityCtl, THREAD_PRIORITY_NORMAL, L"NORMAL");
	AddStringAndItemDataToCombo(m_cmbThreadPriorityCtl, THREAD_PRIORITY_ABOVE_NORMAL, L"ABOVE_NORMAL");
	AddStringAndItemDataToCombo(m_cmbThreadPriorityCtl, THREAD_PRIORITY_HIGHEST, L"HIGHEST");
	if ( CB_ERR != ix )
	{
		m_cmbThreadPriorityCtl.SetCurSel(ix);
	}

	ix = AddStringAndItemDataToCombo(m_cmbCpuBoundFileTouch, (DWORD)eNoFileTouch, L"No file touch");
	AddStringAndItemDataToCombo(m_cmbCpuBoundFileTouch, (DWORD)eOneTimeFileTouch, L"One-time file touch");
	AddStringAndItemDataToCombo(m_cmbCpuBoundFileTouch, (DWORD)eRepeatedFileTouch, L"Repeated file touch");
	if ( CB_ERR != ix )
	{
		m_cmbCpuBoundFileTouch.SetCurSel(ix);
	}

	ix = AddStringAndItemDataToCombo(m_ctlExceptionType, (DWORD)eET_int, L"int");
	AddStringAndItemDataToCombo(m_ctlExceptionType, (DWORD)eET_float, L"float");
	AddStringAndItemDataToCombo(m_ctlExceptionType, (DWORD)eET_stdString, L"std::string");
	AddStringAndItemDataToCombo(m_ctlExceptionType, (DWORD)eET_stdWstring, L"std::wstring");
	AddStringAndItemDataToCombo(m_ctlExceptionType, (DWORD)eET_voidStar, L"void *");
	AddStringAndItemDataToCombo(m_ctlExceptionType, (DWORD)eET_charStar, L"char *");
	AddStringAndItemDataToCombo(m_ctlExceptionType, (DWORD)eET_wcharStar, L"wchar_t *");
	AddStringAndItemDataToCombo(m_ctlExceptionType, (DWORD)eET_customClass, L"custom class");
	AddStringAndItemDataToCombo(m_ctlExceptionType, (DWORD)eET_divByZero, L"divide by zero");
	AddStringAndItemDataToCombo(m_ctlExceptionType, (DWORD)eET_breakpoint, L"breakpoint");
	AddStringAndItemDataToCombo(m_ctlExceptionType, (DWORD)eET_memoryAccessViolation, L"memory access violation");
	if (CB_ERR != ix)
		m_ctlExceptionType.SetCurSel(ix);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVirtMemTestDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVirtMemTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVirtMemTestDlg::OnEndSession(BOOL bEnding)
{
	CDialog::OnEndSession(bEnding);
}

void CVirtMemTestDlg::OnClose()
{
	CDialog::OnClose();
}

void CVirtMemTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}


// ----------------------------------------------------------------------------------------------------
// Memory operations
// ----------------------------------------------------------------------------------------------------

static bool WriteOpcodes(void* pv, SIZE_T nAllocSize)
{
	__try
	{
		// Try to write executable code into allocated memory; ignore if not able to write there.
		size_t ix = 0;
		byte* pBytes = (byte*)pv;
		for (; ix < nAllocSize - 8; ++ix)
		{
			// x86 opcode for NOP; need to figure out what to do for the x64
			// Seems to work for x64 too.
			pBytes[ix] = 0x90;
		}
		// x86 opcode for RET; need to figure out what to do for the x64 version
		// Seems to work for x64 too.
		pBytes[ix] = 0xC3;
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
}

static bool SetProtectionAndWriteOpcodes(void* pv, SIZE_T nAllocSize, std::wstring& sErrorInfo)
{
	bool retval = false;
	sErrorInfo.clear();
	DWORD dwExistingProtection = 0;
	if (!VirtualProtect(pv, nAllocSize, PAGE_READWRITE, &dwExistingProtection))
	{
		DWORD dwLastErr = GetLastError();
		sErrorInfo = std::wstring(L"Couldn't change page protection: ") + SysErrorMessageWithCode(dwLastErr);
		return false;
	}
	else
	{
		retval = WriteOpcodes(pv, nAllocSize);
		if (!retval)
		{
			sErrorInfo = L"Exception while writing executable code";
		}
	}
	if (!VirtualProtect(pv, nAllocSize, dwExistingProtection, &dwExistingProtection))
	{
		DWORD dwLastErr = GetLastError();
		sErrorInfo = std::wstring(L"Couldn't restore page protection: ") + SysErrorMessageWithCode(dwLastErr);
		retval = false;
	}
	return retval;
}

void CVirtMemTestDlg::PostAllocOperations(void* pAllocatedMem, SIZE_T nAllocSize)
{
	// Statically declare the buffer so that we're not creating new allocations as part of this operation.
	const size_t readbufsize = 1024;
	static BYTE readbuf[readbufsize];

	if (m_bRead)
	{
		// Read it in chunks (also, just dereferencing the memory and not doing anything with it
		// might get optimized out)
		for (size_t ix = 0; ix + readbufsize < nAllocSize; ix += readbufsize)
		{
			memcpy(readbuf, ((const BYTE*)pAllocatedMem) + ix, readbufsize);
		}
	}
	if (m_bWrite)
	{
		// Write it in chunks
		for (size_t ix = 0; ix + readbufsize < nAllocSize; ix += readbufsize)
		{
			memcpy(((BYTE*)pAllocatedMem) + ix, readbuf, readbufsize);
		}
	}
	if (m_bExecute)
	{
		typedef void(*pfn_t)();
		pfn_t pfn = (pfn_t)pAllocatedMem;
		pfn();
	}
	if (m_bExecuteNOPs)
	{
		bool bStillExecute = true;
		std::wstring sErrorInfo;
		if (!SetProtectionAndWriteOpcodes(pAllocatedMem, nAllocSize, sErrorInfo))
		{
			std::wstring sQuery = std::wstring(L"Unable to write opcodes\n") + sErrorInfo + L"\nExecute anyway?";
			if (IDNO == MessageBoxW(sQuery.c_str(), NULL, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION))
			{
				bStillExecute = false;
			}
		}
		if (bStillExecute)
		{
			typedef void(*pfn_t)();
			pfn_t pfn = (pfn_t)pAllocatedMem;
			pfn();
		}
	}
}

DWORD CVirtMemTestDlg::SelectedMemoryProtection()
{
	int cmbIx = m_cmbPageProtectionCtl.GetCurSel();
	if (CB_ERR == cmbIx)
	{
		// Shouldn't be possible to hit this.
		return 0;
	}
	return (DWORD)m_cmbPageProtectionCtl.GetItemData(cmbIx);
}

BOOL CVirtMemTestDlg::SetMemoryProtection(void* pv, SIZE_T nAllocSize)
{
	BOOL ret = FALSE;
	DWORD dwProtection = SelectedMemoryProtection();
	if (0 != dwProtection)
	{
		DWORD dwPreviousValue = 0;
		ret = VirtualProtect(pv, nAllocSize, dwProtection, &dwPreviousValue);
		if (!ret)
		{
			DWORD dwLastErr = GetLastError();
			std::wstringstream strQuery;
			strQuery
				<< L"Unable to set memory protection on allocated memory:" << std::endl
				<< SysErrorMessageWithCode(dwLastErr) << std::endl
				<< L"Proceed with post-allocation operations anyway?";
			if (IDYES == MessageBoxW(strQuery.str().c_str(), NULL, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION))
			{
				ret = true;
			}
		}
	}
	return ret;
}

void CVirtMemTestDlg::AddAddrToList(void* addr)
{
	wchar_t buf[32];
	wsprintfW(buf, L"%p", addr);
	m_list.InsertString(0, buf);
	UpdateData(FALSE);
}

void CVirtMemTestDlg::OnBnClickedVirtualAlloc()
{
	UpdateData();

	DWORD dwFlags = MEM_RESERVE;
	if ( m_bCommit )
		dwFlags |= MEM_COMMIT;

	DWORD dwProtection = SelectedMemoryProtection();
	if (0 == dwProtection)
	{
		// Shouldn't be possible to hit this.
		MessageBoxW(L"Please select page protection.");
		return;
	}

	SIZE_T nAllocSize = SIZE_T(m_dwMbToAllocate) * 1024 * 1024;
	LPVOID pv = VirtualAlloc(NULL, nAllocSize, dwFlags, dwProtection);
	if ( NULL != pv )
	{
		AddAddrToList(pv);
		PostAllocOperations(pv, nAllocSize);

		if ( m_bFree )
		{
			MessageBoxW(L"Click OK to free the mem");
			if ( !VirtualFree(pv, 0, MEM_RELEASE) )
			{
				DWORD dwLastErr = GetLastError();
				wstringstream sInfo;
				sInfo << L"VirtualFree failed:\n" << SysErrorMessageWithCode(dwLastErr);
				MessageBoxW(sInfo.str().c_str());
			}
		}
	}
	else
	{
		DWORD dwLastErr = GetLastError();
		wstringstream sInfo;
		sInfo << L"VirtualAlloc failed:\n" << SysErrorMessageWithCode(dwLastErr);
		MessageBoxW(sInfo.str().c_str());
	}
}

void CVirtMemTestDlg::OnBnClickedCreatefilemapping()
{
	UpdateData();
	SharedMem * pSharedMem = new SharedMem;
	ULARGE_INTEGER nAllocSize = { 0 };
	nAllocSize.QuadPart = ULONGLONG(m_dwMbToAllocate) * 1024 * 1024;
	LPVOID pv = pSharedMem->Create(L"Virtual memory test", nAllocSize.LowPart, nAllocSize.HighPart);
	if ( NULL != pv )
	{
		AddAddrToList(pv);
		if (SetMemoryProtection(pv, nAllocSize.QuadPart))
			PostAllocOperations(pv, nAllocSize.QuadPart);

		if ( m_bFree )
		{
			MessageBoxW(L"Click OK to free the mem");
			delete pSharedMem;
		}
	}
	else
	{
		wstringstream sInfo;
		sInfo << L"File mapping failed:\n" << SysErrorMessageWithCode(pSharedMem->ErrorCode());
		MessageBoxW(sInfo.str().c_str());
		delete pSharedMem;
	}
}

void CVirtMemTestDlg::OnBnClickedStack()
{
	UpdateData();
	SIZE_T nAllocSize = SIZE_T(m_dwMbToAllocate) * 1024;
	__try
	{
		LPVOID pv = _alloca(nAllocSize);
		if ( pv )
		{
			AddAddrToList(pv);
			if (SetMemoryProtection(pv, nAllocSize))
				PostAllocOperations(pv, nAllocSize);

			if ( m_bFree )
			{
				MessageBoxW(L"Click OK to free the mem");
			}
		}
		else
		{
			MessageBoxW(L"_alloca returned NULL and didn't raise an exception?!?!");
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		MessageBoxW(L"_alloca failed");
	}
}

void CVirtMemTestDlg::OnBnClickednewbyte()
{
	UpdateData();
	SIZE_T nAllocSize = SIZE_T(m_dwMbToAllocate) * 1024 * 1024;
	__try
	{
		LPBYTE pv = new BYTE[nAllocSize];
		if ( pv )
		{
			AddAddrToList(pv);
			if (SetMemoryProtection(pv, nAllocSize))
				PostAllocOperations(pv, nAllocSize);

			if ( m_bFree )
			{
				MessageBoxW(L"Click OK to free the mem");
				delete[] pv;
			}
		}
		else
		{
			MessageBoxW(L"new[] returned NULL and didn't raise an exception?!?!");
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		MessageBoxW(L"new[] failed");
	}
}

// ----------------------------------------------------------------------------------------------------
// CPU Consumption
// ----------------------------------------------------------------------------------------------------

volatile bool bStop = false;
// Add some nesting so the call stack depth can be different for each snapshot.
// Turn off compiler optimizations here so that the functions aren't all merged into one.
#pragma optimize( "", off )
static bool ThirdLevelNestedFn()
{
	return bStop;
}
static bool SecondLevelNestedFn()
{
	return ThirdLevelNestedFn();
}
static bool ANestedFunction()
{
	return SecondLevelNestedFn();
}
// Restore previous optimizations.
#pragma optimize( "", on )

/// <summary>
/// CPU-intensive thread function
/// </summary>
DWORD WINAPI CpuHogThread(LPVOID pvFileTouch)
{
	eCpuBoundFileTouch_t eFileTouch = (eCpuBoundFileTouch_t)(DWORD)pvFileTouch;
	if ( eNoFileTouch != eFileTouch )
	{
		CloseHandle(CreateFileW(L"C:\\VirtMemTest.log", FILE_GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL));
	}
	int cnt = 0;
	while(!ANestedFunction())
	{
		if (eRepeatedFileTouch == eFileTouch)
		{
			if (++cnt > 0x7FFFFFF)
			{
				cnt = 0;
				CloseHandle(CreateFileW(L"C:\\VirtMemTest.log", FILE_GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL));
			}
		}
	}
	return 0;
}

void CVirtMemTestDlg::OnBnClickedSinglecpu()
{
	UpdateData();
	DWORD_PTR ProcessAffinityMask, SystemAffinityMask, MaskToSet = 0;
	DWORD dwLastErr = 0;
	BOOL ret, bGetFailure = FALSE;
	if (m_bSingleCPU)
	{
		MaskToSet = 1;
		ret = SetProcessAffinityMask(GetCurrentProcess(), MaskToSet);
		dwLastErr = GetLastError();
	}
	else
	{
		ret = GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffinityMask, &SystemAffinityMask);
		dwLastErr = GetLastError();
		if (!ret)
			bGetFailure = TRUE;
		else
		{
			// This might fail after setting thread-to-CPU affinity on a system with multiple processor groups.
			MaskToSet = SystemAffinityMask;
			ret = SetProcessAffinityMask(GetCurrentProcess(), MaskToSet);
			dwLastErr = GetLastError();
		}
	}

	if (!ret)
	{
		wstringstream errtext;
		if (bGetFailure)
			errtext << L"GetProcessAffinityMask failed:\n" << SysErrorMessageWithCode(dwLastErr);
		else
			errtext << L"SetProcessAffinityMask failed setting mask " << HEX(MaskToSet) << L":\n" << SysErrorMessageWithCode(dwLastErr);
		MessageBoxW(errtext.str().c_str());
	}
}

void CVirtMemTestDlg::OnBnClickedThreadToCPUAffinity()
{
	// Do the work when the threads get created.
}

void CVirtMemTestDlg::OnBnClickedCpuhog()
{
	UpdateData();
	if ( m_bHog )
	{
		m_ctlSingleCPU.EnableWindow(FALSE);
		m_ctlThreadToCPUAffinity.EnableWindow(FALSE);
		m_ctlThreadCount.EnableWindow(FALSE);
		m_cmbThreadPriorityCtl.EnableWindow(FALSE);
		m_cmbCpuBoundFileTouch.EnableWindow(FALSE);

		int cmbIx = m_cmbThreadPriorityCtl.GetCurSel();
		if ( CB_ERR == cmbIx )
		{
			MessageBoxW(L"Please select thread priority.");
			return;
		}
		int nPriority = (int)m_cmbThreadPriorityCtl.GetItemData(cmbIx);

		cmbIx = m_cmbCpuBoundFileTouch.GetCurSel();
		if ( CB_ERR == cmbIx )
		{
			MessageBoxW(L"Please select file touch option.");
			return;
		}
		eCpuBoundFileTouch_t eFileTouch = (eCpuBoundFileTouch_t)m_cmbCpuBoundFileTouch.GetItemData(cmbIx);

		// If the system has more than 64 processors, will need to specify processor groups along with thread affinities.
		// How many groups are there?
		WORD procGroupCount = GetActiveProcessorGroupCount();
		std::vector<DWORD> groupProcessorCounts(procGroupCount);
		// How many processors per group?
		for (WORD gnum = 0; gnum < procGroupCount; ++gnum)
		{
			groupProcessorCounts[gnum] = GetActiveProcessorCount(gnum);
		}

		std::wstringstream strErrorInfo;
		bStop = false;
		WORD wCurrGroup = 0, wCurrProcessor = 0;
		for (DWORD ix = 0; ix < m_CpuHogsCount; ++ix)
		{
			// Start the new thread suspended; resume it after setting (optional) processor affinity and priority.
			DWORD tid = 0;
			HANDLE hThread = CreateThread(nullptr, 0, CpuHogThread, (void*)eFileTouch, CREATE_SUSPENDED, &tid);
			if ( hThread )
			{
				bool bResume = true;
				if (m_bThreadToCPUAffinity)
				{
					// Assign each thread to a separate logical processor, starting with group 0, processor 0, then proc 1, proc 2, etc.; if exceeding the number of processors 
					// in the group, start with the next group. After last available processor, start over with group 0, proc 0.
					// (Assumes this process is allowed to set affinity with any processor in any group.)
					GROUP_AFFINITY groupAffinity = { 0 }, formerGroupAffinity = { 0 };
					groupAffinity.Mask = (KAFFINITY)1 << wCurrProcessor;
					groupAffinity.Group = wCurrGroup;
					dbgOut.locked() << L"Setting thread group affinity: group " << wCurrGroup << L"; affinity " << HEX(groupAffinity.Mask) << std::endl;
					if (0 == SetThreadGroupAffinity(hThread, &groupAffinity, &formerGroupAffinity))
					{
						DWORD dwLastErr = GetLastError();
						strErrorInfo << L"SetThreadGroupAffinity failed for thread " << ix << L", group " << wCurrGroup << L" with affinity " << HEX(groupAffinity.Mask) << L": " << SysErrorMessageWithCode(dwLastErr) << std::endl;
						TerminateThread(hThread, 0);
						bResume = false;
					}
					if (++wCurrProcessor >= groupProcessorCounts[wCurrGroup])
					{
						wCurrProcessor = 0;
						if (++wCurrGroup >= procGroupCount)
						{
							wCurrGroup = 0;
						}
					}
				}
				if (bResume && !SetThreadPriority(hThread, nPriority))
				{
					DWORD dwLastErr = GetLastError();
					strErrorInfo << L"SetThreadPriority failed for thread " << ix << L": " << SysErrorMessageWithCode(dwLastErr) << std::endl;
					TerminateThread(hThread, 0);
					bResume = false;
				}
				if (bResume)
					ResumeThread(hThread);
				CloseHandle(hThread);
			}
			else
			{
				DWORD dwLastErr = GetLastError();
				strErrorInfo << L"CreateThread failed for thread " << ix << L": " << SysErrorMessageWithCode(dwLastErr) << std::endl;
			}
		}
		if (strErrorInfo.str().length() > 0)
			MessageBoxW(strErrorInfo.str().c_str(), nullptr, MB_ICONEXCLAMATION);
	}
	else
	{
		bStop = true;
		m_ctlSingleCPU.EnableWindow(TRUE);
		m_ctlThreadToCPUAffinity.EnableWindow(TRUE);
		m_ctlThreadCount.EnableWindow(TRUE);
		m_cmbThreadPriorityCtl.EnableWindow(TRUE);
		m_cmbCpuBoundFileTouch.EnableWindow(TRUE);
	}
}

// ----------------------------------------------------------------------------------------------------
// Hung UI
// ----------------------------------------------------------------------------------------------------

void CVirtMemTestDlg::OnBnClickedHang()
{
	UpdateData();
	Sleep(m_dwHangSecs * 1000);
	MessageBoxW(L"Hang completed");
}

// ----------------------------------------------------------------------------------------------------
// Exceptions
// ----------------------------------------------------------------------------------------------------

class CustomExceptionType
{
public:
	CustomExceptionType(const wchar_t* szExceptionText)
	{
		if (NULL == szExceptionText)
			m_sExceptionText = L"Not supplied";
		else
			m_sExceptionText = szExceptionText;
	}
	~CustomExceptionType()
	{}
	CustomExceptionType(CustomExceptionType& other)
	{
		m_sExceptionText = other.m_sExceptionText;
	}
	CustomExceptionType& operator = (CustomExceptionType& other)
	{
		m_sExceptionText = other.m_sExceptionText;
		return *this;
	}
	const CStringW& Data() const
	{
		return m_sExceptionText;
	}
private:
	CStringW m_sExceptionText;
};


void ThrowException(eExceptionType_t excType)
{
	switch (excType)
	{
	case eET_int:
		throw (int)0x1234;
		break;
	case eET_float:
		throw (float)123.456;
		break;
	case eET_stdString:
		throw std::string("a standard string");
		break;
	case eET_stdWstring:
		throw std::wstring(L"a standard wstring");
		break;
	case eET_voidStar:
		throw (void*)ThrowException;
		break;
	case eET_charStar:
		throw "a char star";
		break;
	case eET_wcharStar:
		throw L"a wchar star";
		break;
	case eET_customClass:
	{
		CustomExceptionType exc(L"A custom class");
		throw exc;
	}
	break;
	case eET_divByZero:
	{
		int i = 0;
		int j = 5 / i;
		// This line never executes; it's here to get rid of the "local variable is initialized but not referenced" warning
		i = j;
	}
	break;
	case eET_breakpoint:
		if (IsDebuggerPresent())
			DebugBreak();
		break;
	case eET_memoryAccessViolation:
	{
		int* pNull = NULL;
		*pNull = 0x1234;
	}
	break;
	default:
		MessageBoxW(NULL, L"Unrecognized exception type", L"ThrowException", MB_ICONERROR);
		break;
	}
}

void CVirtMemTestDlg::OnBnClickedThrowexception()
{
	UpdateData();
	int cmbIx = m_ctlExceptionType.GetCurSel();
	if (CB_ERR == cmbIx)
	{
		MessageBoxW(L"Please select exception type.");
		return;
	}
	eExceptionType_t excType = (eExceptionType_t)m_ctlExceptionType.GetItemData(cmbIx);

	if (m_bCatchException)
	{
		__try
		{
			//try
			{
				ThrowException(excType);
			}
			//catch (...)
			{
				//	MessageBoxW(L"C++ catch");
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			//MessageBoxW(L"SEH catch");
		}
	}
	else
	{
		ThrowException(excType);
	}
}

// ----------------------------------------------------------------------------------------------------
// Miscellanous
// ----------------------------------------------------------------------------------------------------

void CVirtMemTestDlg::OnBnClickedOutputdebugstring()
{
	UpdateData();
	OutputDebugStringW(m_sDebugText.GetString());
}

extern BOOL g_bCrash;
void CVirtMemTestDlg::OnBnClickedCrashonendsession()
{
	UpdateData();
	g_bCrash = m_bCrashOnEndSession;
}




