
// VM ProjectDlg.h : header file
//

#pragma once


// CVMProjectDlg dialog
class CVMProjectDlg : public CDialogEx
{
// Construction
public:
	CVMProjectDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_VMPROJECT_DIALOG };

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
public:
	afx_msg void OnBnClickedSelectfile();
	CString txtLog;
	void AddLog(LPWSTR,bool);
	void AddLog(LPSTR,bool);
	void AddLog(DWORD,bool);
	afx_msg void OnBnClickedDovirtualize();
	CString strVAStart;
	CString strVAEnd;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedExit();
};
