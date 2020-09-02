#pragma once
#include "VMHandlerProcessor.h"
#include "BeaEngine.h"
class CVMCompiler
{
private:
	CVMHandlerProcessor &iVMHandler;
public:
	DWORD VAStart;
	DWORD VAEnd;
	DWORD VMOpcodeBase;
	LPBYTE OpcodeTable;
	DWORD UsedSize;
	DWORD AddressTable[1000][2];
	DWORD AddressTableCount;
	void Finalize();
	void CompileNow(DWORD,LPBYTE,DWORD,DISASM&);


	//------------------------------
	
	void Compile_PUSH_R32(unsigned int RegisterOffset);
	void Compile_POP_R32(unsigned int RegisterOffset);
	void Compile_PUSH_Imm32(DWORD Imm);
	void Compile_AND_32();
	void Compile_NOT_32();
	void Compile_JCCIn();
	void Compile_SHR_32();
	void Compile_SHL_32();
	void Compile_ADD_32();
	void Compile_OR_32();
	void Compile_CMP_32();
	void Compile_SUB_32();
	void Compile_GetDWORDDS();
	void Compile_SetDWORDDS();
	void Compile_NOTBit();


	void Process_JCC(LPBYTE Opcode,DWORD Dest);
	void ProcessPUSHR32(BYTE iOpcode);
	void ProcessPOPR32(BYTE iOpcode);
	void ProcessMOVR32R32(LPBYTE iOpcode);
	void ProcessADDR32_Imm(LPBYTE iOpcode);
	void ProcessSUBR32_Imm(LPBYTE iOpcode);
	void ProcessMOV_MemR32Imm8_R32(LPBYTE iOpcode);
	void ProcessMOV_MemR32Imm32_R32(LPBYTE iOpcode);
	void ProcessMOVR32_Imm(LPBYTE iOpcode);
	void ProcessMOVR32_MemImm(LPBYTE iOpcode);
	void ProcessMOVMemImm_R32(LPBYTE iOpcode);
	void ProcessMOV_MemR32Imm32_Imm32(LPBYTE iOpcode);
	void ProcessMOV_R32_MemR32Imm32(LPBYTE iOpcode);
	void ProcessMOV_R32_MemR32Imm8(LPBYTE iOpcode);
	void ProcessLEA_R32_MemR32Imm32(LPBYTE iOpcode);
	void ProcessXORR32_R32(LPBYTE iOpcode);
	void ProcessANDR32_Imm(LPBYTE iOpcode);
	void ProcessORR32_Imm(LPBYTE iOpcode);
	void ProcessSHLR32_Imm(LPBYTE iOpcode);
	void ProcessSHRR32_Imm(LPBYTE iOpcode);
	void ProcessCMPR32_Imm(LPBYTE iOpcode);
	void ProcessNOTR32(LPBYTE iOpcode);
	void ProcessINC_DECR32(LPBYTE iOpcode);



	//-------------------------------

	CVMCompiler(CVMHandlerProcessor &HandlerProcessor);
	~CVMCompiler(void);
};

