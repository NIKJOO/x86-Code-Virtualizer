#pragma once

enum VMHandlerCodes{VM_X86,VM_CALL,VM_Exit,VM_Start,VM_PUSHImmDW,VM_JCCIn,VM_PUSHR32,VM_POPR32,VM_AND32,VM_NOT32,VM_SHR32,VM_SHL32,VM_ADD32,VM_PUSHESP,VM_POPESP,VM_SetDWORDDS,VM_GetDWORDDS};

class CVMHandlerProcessor
{
private:
	DWORD HandlerTable[256];
	DWORD HandlerCurrentTable[256];
	DWORD HandlerCount;
	DWORD GetHandlerSize(LPBYTE);
	DWORD dwVMHandlerTableAddress;
	DWORD GetMainAddress(LPBYTE);
public:
	DWORD dwVMEntryAddress;
	
	CVMHandlerProcessor(void);
	~CVMHandlerProcessor(void);
	unsigned int GetHandlerIndex(VMHandlerCodes);
	DWORD BuildVMEngine(DWORD,LPBYTE);

	//-------VM Handlers
	void VMH_X86();
	void VMH_CALL();
	void VMH_Exit();
	void VMH_PUSHImmDWORD();
	void VMH_JCCIn();
	void VMH_Start();
	void VMH_Dispatcher();
	void VMH_PUSHR32();
	void VMH_POPR32();
	void VMH_AND32();
	void VMH_NOT32();
	void VMH_SHR32();
	void VMH_SHL32();
	void VMH_ADD32();
	void VMH_PUSHESP();
	void VMH_POPESP();
	void VMH_SetDWORDDS();
	void VMH_GetDWORDDS();
};

