#include "stdafx.h"
#include "InstructionLogger.h"


// Just log all translated instructions ...


CInstructionLogger::CInstructionLogger(CVMHandlerProcessor &HandlerProcessor):iVMHandler(HandlerProcessor)
{
	
	LogCount=0;
	CareCount=0;
	pOriginalVATable=(PDWORD)malloc(TableMemorySize);
	pVMVATable=(PDWORD)malloc(TableMemorySize);
	pMemTable=(PDWORD)malloc(TableMemorySize);
	pCareTargetAddr=(PDWORD)malloc(TableMemorySize);
	pCareVMMem=(PDWORD)malloc(TableMemorySize);
}
CInstructionLogger::~CInstructionLogger(void)
{
}
void CInstructionLogger::Clear()
{
	CareCount=0;
	LogCount=0;
	ZeroMemory(pOriginalVATable,TableMemorySize);
	ZeroMemory(pVMVATable,TableMemorySize);
	ZeroMemory(pMemTable,TableMemorySize);
	ZeroMemory(pCareTargetAddr,TableMemorySize);
	ZeroMemory(pCareVMMem,TableMemorySize);
}
void CInstructionLogger::AddToLog(DWORD OriginalVirtualAddr,DWORD VMAddr,LPBYTE MemByte)
{
	pOriginalVATable[LogCount]=OriginalVirtualAddr;
	pVMVATable[LogCount]=VMAddr;
	pMemTable[LogCount]=(DWORD)MemByte;
	LogCount++;

}
void CInstructionLogger::AddToCare(LPBYTE MemVM,DWORD TargetVAddr)
{
	pCareVMMem[CareCount]=(DWORD)MemVM;
	pCareTargetAddr[CareCount]=TargetVAddr;
	CareCount++;
}
LPBYTE CInstructionLogger::GetMemFromVA(DWORD VirtualAddr)
{
	LPBYTE iResult=(LPBYTE)0xFFFFFFFF;

	for (DWORD i=0;i<LogCount;i++)
		if (pOriginalVATable[i]==VirtualAddr)
		{
			iResult=(LPBYTE)pMemTable[i];
			break;
		}


		return iResult;
}
void CInstructionLogger::FixCare()
{
	LPBYTE iOpCode;
	DWORD OriginalVA;
	DWORD TargetVA;
	DWORD FoundVMVA;
	for (DWORD i=0;i<CareCount;i++)
	{
		iOpCode=(LPBYTE)pCareVMMem[i];
		TargetVA=pCareTargetAddr[i];

		for (DWORD j=0;j<LogCount;j++)
			if (pOriginalVATable[j]==TargetVA)
			{
				FoundVMVA=pVMVATable[j];
				break;
			}

				*(LPDWORD(&iOpCode[1]))=FoundVMVA;
				



	}

}