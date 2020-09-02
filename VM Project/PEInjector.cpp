#include "stdafx.h"
#include "PEInjector.h"

// This Class Used to Read PE data and add some data to it
// this data can stored as new section or any thing else

CPEInjector::CPEInjector(void)
{
	pPEBytes=0;
	pOverlay=0;
	OverlaySize=0;
	PEFileSize=0;
	
}


CPEInjector::~CPEInjector(void)
{
}

void CPEInjector::LoadFile(LPWSTR iFile)
{
	if (pPEBytes)
		delete[] pPEBytes;
	if (pOverlay)
		delete[] pOverlay;
	HANDLE hFile;
	DWORD tmp;

	hFile=CreateFileW(iFile,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
	PEFileSize=GetFileSize(hFile,0);
	pPEBytes=(LPBYTE)malloc(PEFileSize);
	ReadFile(hFile,pPEBytes,PEFileSize,&tmp,0);
	CloseHandle(hFile);

	DOSHeader=(PIMAGE_DOS_HEADER)pPEBytes;
	NTHeader=(PIMAGE_NT_HEADERS)(pPEBytes + DOSHeader->e_lfanew);
	SectionHeader=(PIMAGE_SECTION_HEADER)((LPBYTE)&(NTHeader->OptionalHeader) + NTHeader->FileHeader.SizeOfOptionalHeader);


}
DWORD CPEInjector::GetNewSectionVA()
{
	DWORD NewVA;
	NewVA=SectionHeader[NTHeader->FileHeader.NumberOfSections-1].VirtualAddress +  SectionHeader[NTHeader->FileHeader.NumberOfSections-1].Misc.VirtualSize;
	NewVA=AlignSize(NewVA,NTHeader->OptionalHeader.SectionAlignment);
	NewVA+=NTHeader->OptionalHeader.ImageBase;
	return NewVA;

}
DWORD CPEInjector::GetNewSectionOffset()
{
	DWORD NewOffset;
	NewOffset=SectionHeader[NTHeader->FileHeader.NumberOfSections-1].PointerToRawData +  SectionHeader[NTHeader->FileHeader.NumberOfSections-1].SizeOfRawData;
	NewOffset=AlignSize(NewOffset,NTHeader->OptionalHeader.FileAlignment);
	
	return NewOffset;

}
void CPEInjector:: AddNewSection(LPBYTE SectionMem,size_t MemSize)
{
	char SectionName[8]=".XVM";
	DWORD NewRVA=GetNewSectionVA()-NTHeader->OptionalHeader.ImageBase;
	DWORD NewOffset=GetNewSectionOffset();
	DWORD NewSize=AlignSize(MemSize,NTHeader->OptionalHeader.SectionAlignment);
	DWORD i=NTHeader->FileHeader.NumberOfSections;

	SectionHeader[i].Characteristics=0xE00000A0;
	SectionHeader[i].Misc.VirtualSize=NewSize;
	SectionHeader[i].SizeOfRawData=NewSize;
	SectionHeader[i].VirtualAddress=NewRVA;
	SectionHeader[i].PointerToRawData=NewOffset;
	CopyMemory(SectionHeader[i].Name,SectionName,8);

	NTHeader->FileHeader.NumberOfSections++;
	NTHeader->OptionalHeader.SizeOfImage+=NewSize;

	NewSectionMem=SectionMem;
	NewSectionSize=MemSize;

}
void CPEInjector:: SaveFile(LPSTR destAddr)
{
	HANDLE hFile;
	DWORD tmp;
	hFile=CreateFileA(destAddr,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,0,0);
	WriteFile(hFile,pPEBytes,PEFileSize,&tmp,0);
	WriteFile(hFile,NewSectionMem,NewSectionSize,&tmp,0);
	CloseHandle(hFile);
}
DWORD CPEInjector:: AlignSize(DWORD dwSize, DWORD dwAlign) 
{
	return (dwSize + (dwAlign - ((dwSize % dwAlign) ? (dwSize % dwAlign) : dwAlign)));
}
DWORD CPEInjector::FindSectionNum(DWORD VA)
{
	VA-=NTHeader->OptionalHeader.ImageBase;

	for (int i=0;i<NTHeader->FileHeader.NumberOfSections;i++)
	{
		if (VA>=SectionHeader[i].VirtualAddress && VA<(SectionHeader[i].VirtualAddress + SectionHeader[i].Misc.VirtualSize))
			return i;
	}

	return -1;

}
DWORD CPEInjector::V2O(DWORD VA)
{
	
	int i=FindSectionNum(VA);
	if (i!=-1)
	{
		VA-=NTHeader->OptionalHeader.ImageBase;
		VA-=SectionHeader[i].VirtualAddress;
		VA+=SectionHeader[i].PointerToRawData;
		return VA;
	}

	return -1;
}