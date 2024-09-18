
// VirtMemTestDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CVirtMemTestDlg dialog
class CVirtMemTestDlg : public CDialog
{
// Construction
public:
	CVirtMemTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_VIRTMEMTEST_DIALOG };

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

private:
	DWORD m_dwMbToAllocate;
	BOOL m_bCommit;
	BOOL m_bFree;
	BOOL m_bWrite;
	BOOL m_bHog;
	BOOL m_bSingleCPU;
	BOOL m_bThreadToCPUAffinity;
	DWORD m_dwHangSecs;
	BOOL m_bCrashOnEndSession;
	BOOL m_bExecute;
	BOOL m_bExecuteNOPs;
	BOOL m_bRead;
	DWORD m_CpuHogsCount;

	CListBox m_list;
	CComboBox m_cmbPageProtectionCtl;
	CComboBox m_cmbThreadPriorityCtl;
	CEdit m_ctlThreadCount;
	CButton m_ctlSingleCPU;
	CButton m_ctlThreadToCPUAffinity;
	CComboBox m_cmbCpuBoundFileTouch;
	BOOL m_bCatchException;
	CComboBox m_ctlExceptionType;
	CString m_sDebugText;

	afx_msg void OnBnClickedVirtualAlloc();
	afx_msg void OnBnClickedCreatefilemapping();
	afx_msg void OnBnClickedStack();
	afx_msg void OnBnClickednewbyte();
	void AddAddrToList(void* addr);

	afx_msg void OnBnClickedSinglecpu();
	afx_msg void OnBnClickedThreadToCPUAffinity();
	afx_msg void OnBnClickedCpuhog();
	afx_msg void OnBnClickedHang();
	afx_msg void OnBnClickedCrashonendsession();
	afx_msg void OnBnClickedOutputdebugstring();
	afx_msg void OnBnClickedThrowexception();

	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg void OnClose();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);

	void PostAllocOperations(void* pAllocatedMem, SIZE_T nAllocSize);
	DWORD SelectedMemoryProtection();
	BOOL SetMemoryProtection(void* pv, SIZE_T nAllocSize);
};
