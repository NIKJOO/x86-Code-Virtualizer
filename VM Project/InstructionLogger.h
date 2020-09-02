#pragma once
#include"VMHandlerProcessor.h"
#include <vector>
#define TableMemorySize 0x10000
class CInstructionLogger
{
private:
	CVMHandlerProcessor &iVMHandler;
public:

	DWORD LogCount;
	DWORD CareCount;

	LPDWORD pOriginalVATable;
	LPDWORD pVMVATable;
	LPDWORD pMemTable;

	LPDWORD pCareVMMem;
	LPDWORD pCareTargetAddr;

	void Clear();
	void AddToLog(DWORD OriginalVirtualAddr,DWORD VMAddr,LPBYTE MemByte);
	void AddToCare(LPBYTE MemVM,DWORD TargetVAddr);
	LPBYTE GetMemFromVA(DWORD VirtualAddr);

	CInstructionLogger(CVMHandlerProcessor &HandlerProcessor);
	~CInstructionLogger(void);
	void FixCare();
};

