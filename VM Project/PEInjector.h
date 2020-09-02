#pragma once
#include <iostream>
#include <Windows.h>

using namespace std;

class CPEInjector
{
public:
	LPWSTR strFileName;
	CPEInjector(void);
	~CPEInjector(void);
	void LoadFile(LPWSTR);
	DWORD GetNewSectionVA();
	DWORD GetNewSectionOffset();
	void AddNewSection(LPBYTE,size_t Size);
	DWORD AlignSize(DWORD,DWORD);
	void SaveFile(LPSTR);
	DWORD FindSectionNum(DWORD VA);
	DWORD V2O(DWORD VA);
	LPBYTE pPEBytes;
	size_t PEFileSize;
	LPBYTE pOverlay;
	size_t OverlaySize;
	LPBYTE NewSectionMem;
	size_t NewSectionSize;

	PIMAGE_DOS_HEADER DOSHeader;
	PIMAGE_NT_HEADERS NTHeader;
	PIMAGE_SECTION_HEADER SectionHeader;
	
	
};

