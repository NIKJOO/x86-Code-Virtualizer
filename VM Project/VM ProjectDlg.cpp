
// VM ProjectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VM Project.h"
#include "VM ProjectDlg.h"
#include "afxdialogex.h"
#include "PEInjector.h"
#include "VMHandlerProcessor.h"
#include "BeaEngine.h"
#include "VMCompiler.h"

CPEInjector InjectionEngine;
CVMHandlerProcessor VMHProcessor;
CVMCompiler iVMCompiler(VMHProcessor);
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVMProjectDlg dialog



CVMProjectDlg::CVMProjectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVMProjectDlg::IDD, pParent)
	, txtLog(_T(""))
	, strVAStart(_T(""))
	, strVAEnd(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVMProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_Log, txtLog);
	DDX_Text(pDX, IDC_VAStart, strVAStart);
	DDX_Text(pDX, IDC_VAeND, strVAEnd);
}

BEGIN_MESSAGE_MAP(CVMProjectDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SelectFile, &CVMProjectDlg::OnBnClickedSelectfile)
	ON_BN_CLICKED(IDC_DoVirtualize, &CVMProjectDlg::OnBnClickedDovirtualize)
	ON_BN_CLICKED(IDC_Exit, &CVMProjectDlg::OnBnClickedExit)
END_MESSAGE_MAP()


// CVMProjectDlg message handlers

BOOL CVMProjectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	strVAStart="0x";
	strVAEnd="0x";
	UpdateData(false);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVMProjectDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVMProjectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CVMProjectDlg::OnBnClickedSelectfile()
{
    // Select Standard PE File and read some information from it ....
	// Read OEP , NumberOfSections and Section Names
	UpdateData(true);
	OPENFILENAMEW op;
	CFileDialog* ofd = new CFileDialog(TRUE, NULL, NULL, OFN_FILEMUSTEXIST| OFN_HIDEREADONLY , _T("Executable Files (*.exe)|*.exe|Dynamic-Link Libraries (*.dll)|*.dll|All Files (*.*)|*.*||"), NULL, 0);
	
	if(ofd->DoModal() == IDOK)
	{
		op = ofd->GetOFN();
		InjectionEngine.strFileName=op.lpstrFile;
		InjectionEngine.LoadFile(op.lpstrFile);
		txtLog="";
		AddLog("OEP: ",false);
		AddLog(InjectionEngine.NTHeader->OptionalHeader.ImageBase + InjectionEngine.NTHeader->OptionalHeader.AddressOfEntryPoint,true);
		AddLog("Number Of Section: ",false);
		AddLog(InjectionEngine.NTHeader->FileHeader.NumberOfSections,true);
		for (int i=0;i<InjectionEngine.NTHeader->FileHeader.NumberOfSections;i++)
		{
			AddLog("------->",false);
			AddLog((LPSTR)InjectionEngine.SectionHeader[i].Name,true);
		}

		
	}
}


void CVMProjectDlg::AddLog(LPWSTR iLine,bool GoNextLine)
{
    // Add text log to memo object
	txtLog+=(iLine);
	if (GoNextLine)
	txtLog+=(L"\r\n");
	UpdateData(0);
}
void CVMProjectDlg::AddLog(LPSTR iLine,bool GoNextLine)
{
    // Add text log to memo object
	txtLog+=(iLine);
	if (GoNextLine)
	txtLog+=("\r\n");
	UpdateData(0);
}
void CVMProjectDlg::AddLog(DWORD iNUM,bool GoNextLine)
{
    // Add text log to memo object
	char iLine[12];
	wsprintfA(iLine,"%08X",iNUM);
	//CharUpperA(iLine);
	txtLog+=(iLine);
	if (GoNextLine)
	txtLog+=("\r\n");
	UpdateData(0);
}

void CVMProjectDlg::OnBnClickedDovirtualize()
{
    // Start of Virtualization Routine
	if (InjectionEngine.strFileName == LPWSTR (NULL) )
	{
		MessageBoxA(0,"Can't Open Target File.","Error", MB_OK && MB_ICONERROR);
		return;
	}
    
	// New Section information (.VM Section)
	const DWORD SectionSize=0x10000; 
	BYTE tmpBytes[16];
	LPBYTE NewSection=(LPBYTE)VirtualAlloc(0,SectionSize,0x3000,0x40);
	ZeroMemory(NewSection,SectionSize);
	DWORD NewSectionVA=InjectionEngine.GetNewSectionVA();
	DWORD iCaveVA=NewSectionVA;
	LPBYTE iCaveMem=NewSection;

	// Add new empty section to file
	InjectionEngine.AddNewSection(NewSection,SectionSize);

	
	UpdateData(true);
	DWORD VAStart=wcstol(strVAStart.GetBuffer(), NULL, 16);
	DWORD VAEnd=wcstol(strVAEnd.GetBuffer(), NULL, 16);;
	DWORD iVA=VAStart;
	LPBYTE iCode=InjectionEngine.pPEBytes + InjectionEngine.V2O(iVA);
	DISASM iDiASM;
	int iSize;
	ZeroMemory(&iDiASM,sizeof(DISASM));
	iDiASM.VirtualAddr=iVA;
	iDiASM.EIP=(uintptr_t)iCode;

	// Get Virtual Addresses 
	iVMCompiler.VAStart=VAStart;
	iVMCompiler.VAEnd=VAEnd;
	iVMCompiler.VMOpcodeBase=NewSectionVA;
	
	while (iVA<VAEnd)
	{
	    // Convert Asm instructions to New Byte Codes (Compiling)
		iSize=Disasm(&iDiASM);
		iVMCompiler.CompileNow(iVA,(LPBYTE)(iDiASM.EIP),iSize,iDiASM);


		iVA+=iSize;
		iDiASM.VirtualAddr+=iSize;
		iDiASM.EIP+=iSize;
		
	}
	
	tmpBytes[0]=0x68;
	*(LPDWORD(tmpBytes+1))=iVA;
	iVMCompiler.CompileNow(iVA,tmpBytes,5,iDiASM);
	
    // Finish Compiling...
	tmpBytes[0]=0xC3;
	iVMCompiler.CompileNow(iVA,tmpBytes,1,iDiASM);





	iVMCompiler.Finalize();
	// init VM Handlers for Generated VM Opcodes
	CopyMemory(iCaveMem,iVMCompiler.OpcodeTable,iVMCompiler.UsedSize);
	iCaveMem+=iVMCompiler.UsedSize;
	iCaveVA+=iVMCompiler.UsedSize;
	// Finish Building...
	iSize=VMHProcessor.BuildVMEngine(iCaveVA,iCaveMem);
	iCaveMem+=iSize;
	iCaveVA+=iSize;
	
	
	FillMemory(InjectionEngine.pPEBytes + InjectionEngine.V2O(VAStart),iVA-VAStart,0x90);
	
	{
	    // Add VM Codes & Handlers to PE
		LPBYTE iByte=InjectionEngine.V2O(VAStart) +  InjectionEngine.pPEBytes;
		LPDWORD iDWORD=LPDWORD(iByte+1);

		iByte[0]=0x68;
		*iDWORD=iVMCompiler.VMOpcodeBase;

		iByte+=5;
		iDWORD=LPDWORD(iByte+1);
		iByte[0]=0xE9;
		*iDWORD=(VMHProcessor.dwVMEntryAddress-VAStart-10);
		
	}

	

	

	



    // Save Final File...
	InjectionEngine.SaveFile("C:\\Obfuscated.exe");

	

}


void CVMProjectDlg::OnBnClickedExit()
{
	ExitProcess (0);
	// TODO: Add your control notification handler code here
}
