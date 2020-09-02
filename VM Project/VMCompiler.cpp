#include "stdafx.h"
#include "VMCompiler.h"
#include "InstructionLogger.h"


// This Class will Translate input opcode to VM opcode

CInstructionLogger *InstructionLog;

CVMCompiler::CVMCompiler(CVMHandlerProcessor &HandlerProcessor):iVMHandler(HandlerProcessor)
{
	InstructionLog=new CInstructionLogger(HandlerProcessor);
	AddressTableCount=0;
	UsedSize=0;
	OpcodeTable=(LPBYTE)malloc(0x1000);
	ZeroMemory(OpcodeTable,0x1000);
	
}


CVMCompiler::~CVMCompiler(void)
{
}


void CVMCompiler::CompileNow(DWORD Addr,LPBYTE Opcode,DWORD iSize,DISASM &iDiASM)
{
    // Get Address and byte Code then Generate VM Opcode...
	InstructionLog->AddToLog(Addr,VMOpcodeBase+UsedSize,&OpcodeTable[UsedSize]);


	//------0:CALL Imm
	//------1: x86
	bool bHandled=false;
	AddressTable[AddressTableCount][0]=Addr;
	AddressTable[AddressTableCount][1]=UsedSize;

	LPBYTE iOpcode=&OpcodeTable[UsedSize];
	DWORD iVMOpcode=VMOpcodeBase+UsedSize;
	DWORD tmp;
	DWORD T1=0;
	DWORD T2=0;
	switch (Opcode[0])
	{
	case 0xE8:
		iOpcode[0]=iVMHandler.GetHandlerIndex(VM_CALL);
		tmp=*(LPDWORD(&Opcode[1])) + Addr + 5;
		*(LPDWORD(&iOpcode[1]))=tmp;
		UsedSize+=5;
		break;
	
	case 0x72: 
	case 0x73: 
	case 0x74: 
	case 0x75: 
	case 0x76: 
	case 0x77:
	case 0x7C: 
	case 0x7D: 
	case 0x7E: 
	case 0x7F: 
	case 0xEB:
		T1=iDiASM.Instruction.AddrValue;
		Process_JCC(Opcode,T1);
		break;
	
		
	
	case 0x68:
		tmp=*(LPDWORD(&Opcode[1]));
		Compile_PUSH_Imm32(tmp);
		break;
	case 0x6A:
		tmp=*(LPDWORD(&Opcode[1]));
		tmp&=0xFF;
		__asm
		{
			push eax
				mov eax,tmp
				movsx eax,al
				mov tmp,eax
			pop eax
		}
		Compile_PUSH_Imm32(tmp);
		
		break;


	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
		ProcessPUSHR32(Opcode[0]);
		break;
	case 0x58:
	case 0x59:
	case 0x5A:
	case 0x5B:
	case 0x5C:
	case 0x5D:
	case 0x5E:
	case 0x5F:
		ProcessPOPR32(Opcode[0]);
		break;
	//------------------------------
	default:
		if ((Opcode[0]==0xF && (Opcode[1]==0x82 || Opcode[1]==0x83 || Opcode[1]==0x84 || Opcode[1]==0x85 || Opcode[1]==0x86 || Opcode[1]==0x87|| Opcode[1]==0x8C || Opcode[1]==0x8D ||Opcode[1]==0x8E || Opcode[1]==0x8F)) || Opcode[0]==0xE9)
		{
			
			T1=iDiASM.Instruction.AddrValue;
			Process_JCC(Opcode,T1);

		}
		else if ((Opcode[0]==0x89 || Opcode[0]==0x8B) && Opcode[1]>=0xC0 && Opcode[1]<=0xFF ) 
		{
			ProcessMOVR32R32(Opcode);
		}
		else if ((Opcode[0]==0x81 || Opcode[0]==0x83) && Opcode[1]>=0xC0 && Opcode[1]<0xC8 ) 
		{
			ProcessADDR32_Imm(Opcode);
		}
		else if (Opcode[0]>=0x40 && Opcode[0]<0x50)
		{
			ProcessINC_DECR32(Opcode);
		}
		else if ((Opcode[0]==0x81 || Opcode[0]==0x83) && Opcode[1]>=0xE8 && Opcode[1]<0xF0 ) 
		{
			ProcessSUBR32_Imm(Opcode);
		}
		else if (Opcode[0]==0x89 &&Opcode[1]>=0x40 && Opcode[1]<0x80 ) 
		{
			ProcessMOV_MemR32Imm8_R32(Opcode);
		}
		else if (Opcode[0]==0x89 &&Opcode[1]>=0x40 && Opcode[1]<0xC0 ) 
		{
			ProcessMOV_MemR32Imm32_R32(Opcode);
		}
		else if (Opcode[0]==0xC7 &&Opcode[1]>=0x80 && Opcode[1]<0x88 ) 
		{
			ProcessMOV_MemR32Imm32_Imm32(Opcode);
		}
		else if (Opcode[0]==0x8B &&Opcode[1]>=0x80 && Opcode[1]<0xC0 ) 
		{
			ProcessMOV_R32_MemR32Imm32(Opcode);
		}
		else if (Opcode[0]==0x8B &&Opcode[1]>=0x40 && Opcode[1]<0x80 ) 
		{
			ProcessMOV_R32_MemR32Imm8(Opcode);
		}
		else if (Opcode[0]==0x8D && Opcode[1]>=0x80 && Opcode[1]<0xC0 ) 
		{
			ProcessLEA_R32_MemR32Imm32(Opcode);
		}
		else if ((Opcode[0]==0x33 || Opcode[0]==0x31) && Opcode[1]>=0xC0 && Opcode[1]<=0xFF )
		{
			ProcessXORR32_R32(Opcode);
		}
		else if (Opcode[0]>=0xB8 && Opcode[0]<=0xBF) 
		{
			ProcessMOVR32_Imm(Opcode);
		}
		else if (Opcode[0]==0xA1 || ((Opcode[0]==0x8B) && (Opcode[1]==0xD || Opcode[1]==0x15 || Opcode[1]==0x1D ||  Opcode[1]==0x25 || Opcode[1]==0x2D || Opcode[1]==0x35 || Opcode[1]==0x3D ))) 
		{
			ProcessMOVR32_MemImm(Opcode);
		}
		else if (Opcode[0]==0xA3 || ((Opcode[0]==0x89) && (Opcode[1]==0xD || Opcode[1]==0x15 || Opcode[1]==0x1D ||  Opcode[1]==0x25 || Opcode[1]==0x2D || Opcode[1]==0x35 || Opcode[1]==0x3D ))) 
		{
			ProcessMOVMemImm_R32(Opcode);
		}
		else if (Opcode[0]==0x25 || ((Opcode[0]==0x81) && Opcode[1]>=0xE0 && Opcode[1]<0xE8  )) 
		{
			ProcessANDR32_Imm(Opcode);
		}
		else if (Opcode[0]==0xD || ((Opcode[0]==0x83 || Opcode[0]==0x81) && Opcode[1]>=0xC8 && Opcode[1]<0xD0 )) 
		{
			ProcessORR32_Imm(Opcode);
		}
		else if (Opcode[0]==0x3D || ((Opcode[0]==0x83 || Opcode[0]==0x81) && Opcode[1]>=0xF8 && Opcode[1]<=0xFF )) 
		{
			ProcessCMPR32_Imm(Opcode);
		}
		else if (Opcode[0]==0xC1  && Opcode[1]>=0xE0 && Opcode[1]<0xE8  ) 
		{
			ProcessSHLR32_Imm(Opcode);
		}
		else if (Opcode[0]==0xC1  && Opcode[1]>=0xE8 && Opcode[1]<0xF0  ) 
		{
			ProcessSHRR32_Imm(Opcode);
		}
		else if (Opcode[0]==0xF7 && Opcode[1]>=0xD0 && Opcode[1]<0xD8) 
		{
			ProcessNOTR32(Opcode);
		}
		else
		{
			iOpcode[0]=iVMHandler.GetHandlerIndex(VM_X86);
			iOpcode[1]=iSize;
			CopyMemory(&iOpcode[2],Opcode,iSize);
			UsedSize+=iSize+2;
		}
		break;
	}
	
	
	
}


void CVMCompiler::Finalize()
{
	InstructionLog->FixCare();
}


void CVMCompiler::Compile_PUSH_Imm32(DWORD Imm)
{
     // Generate VM Opcode of "Push"
	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_PUSHImmDW);
	*(LPDWORD(&OpcodeTable[UsedSize+1]))=Imm;
	UsedSize+=5;

}
void CVMCompiler::Compile_PUSH_R32(unsigned int RegisterOffset)
{
     // Generate VM Opcode of "Push Register"
	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_PUSHR32);
	OpcodeTable[UsedSize+1]=RegisterOffset;
	UsedSize+=2;

}
void CVMCompiler::Compile_POP_R32(unsigned int RegisterOffset)
{
     // Generate VM Opcode of "POP"
	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_POPR32);
	OpcodeTable[UsedSize+1]=RegisterOffset;
	UsedSize+=2;
}
void CVMCompiler::Compile_AND_32()
{
     // Generate VM Opcode of "AND"
	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_AND32);
	UsedSize++;
}
void CVMCompiler::Compile_NOT_32()
{
     // Generate VM Opcode of "NOT"
	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_NOT32);
	UsedSize++;
}
void CVMCompiler::Compile_SHL_32()
{
     // Generate VM Opcode of "SHL"
	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_SHL32);
	UsedSize++;
}
void CVMCompiler::Compile_SHR_32()
{
     // Generate VM Opcode of "SHR"
	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_SHR32);
	UsedSize++;
}
void CVMCompiler::Compile_JCCIn()
{
     // Generate VM Opcode of "JCC"
	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_JCCIn);
	UsedSize++;
}
void CVMCompiler::Compile_ADD_32()
{
     // Generate VM Opcode of "ADD"
	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_ADD32);
	UsedSize++;
}
void CVMCompiler::Compile_OR_32()
{
     // Generate VM Opcode of "OR"
	Compile_NOT_32();
	Compile_POP_R32(0x24);
	Compile_NOT_32();
	Compile_AND_32();
	Compile_POP_R32(0x28);
	Compile_NOT_32();
	Compile_POP_R32(0x24);
	Compile_PUSH_R32(0x24);
	Compile_PUSH_R32(0x24);
	Compile_AND_32();


}
void CVMCompiler::Compile_CMP_32()
{
     // Generate VM Opcode of "CMP"
	Compile_SUB_32();
	Compile_POP_R32(0x24);
	Compile_POP_R32(0x28);
	Compile_PUSH_R32(0x24);
}
void CVMCompiler::Compile_SUB_32()
{
     // Generate VM Opcode of "SUB"
	Compile_NOT_32();
	Compile_PUSH_Imm32(1);
	Compile_ADD_32();
	Compile_POP_R32(0x24);
	Compile_ADD_32();
}
void CVMCompiler::Compile_NOTBit()
{
     // Generate VM Opcode of "NOT" Bit
	Compile_PUSH_Imm32(1);
	Compile_ADD_32();
	Compile_POP_R32(0x28);
	Compile_PUSH_Imm32(1);
	Compile_AND_32();
	Compile_POP_R32(0x28);

}
void CVMCompiler::Compile_GetDWORDDS()
{
     // Generate VM Opcode of "GetDWORDS" (all type of opcodes that get DWORD data)
	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_GetDWORDDS);
	UsedSize++;
}
void CVMCompiler::Compile_SetDWORDDS()
{
     // Generate VM Opcode of "GetDWORDS" (all type of opcodes that set DWORD data)
	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_SetDWORDDS);
	UsedSize++;
}

void CVMCompiler::Process_JCC(LPBYTE Opcode,DWORD Dest)
{
     // Generate VM Opcode of "JCC"
	DWORD *dwNext;
	if (Dest>=VAStart && Dest<VAEnd)
	{
			InstructionLog->AddToCare((LPBYTE)(OpcodeTable+UsedSize),Dest);
			Compile_PUSH_Imm32(Dest);
			dwNext=(LPDWORD)&OpcodeTable[UsedSize+1];
			Compile_PUSH_Imm32(0);
		
		if (Opcode[0]==0xEB || Opcode[0]==0xE9) 
		{
			Compile_PUSH_Imm32(1);
			Compile_JCCIn();
		}
		else if (Opcode[0]==0x77 || Opcode[0]==0x76 || (Opcode[0]==0xF && (Opcode[1]==0x87 || Opcode[1]==0x86))) 
		{
			

			Compile_PUSH_R32(0x20);
			Compile_PUSH_Imm32(1);
			Compile_AND_32();
			Compile_POP_R32(0x24);
			Compile_NOTBit();

			Compile_PUSH_R32(0x20);
			Compile_PUSH_Imm32(0x40);
			Compile_AND_32();
			Compile_POP_R32(0x24);
			Compile_PUSH_R32(6);
			Compile_SHR_32();
			Compile_POP_R32(0x24);
			Compile_NOTBit();

			Compile_AND_32();
			Compile_POP_R32(0x24);

			if (Opcode[0]==0x76 || (Opcode[0]==0xF && Opcode[1]==0x86))
				Compile_NOTBit();

			Compile_JCCIn();

			*dwNext=VMOpcodeBase+UsedSize;

		}
		else if (Opcode[0]==0x75 || Opcode[0]==0x74 || (Opcode[0]==0xF && (Opcode[1]==0x84 ||Opcode[1]==0x85 )) ) 
		{
		

			Compile_PUSH_R32(0x20);
			Compile_PUSH_Imm32(0x40);
			Compile_AND_32();
			Compile_POP_R32(0x24);
			Compile_PUSH_Imm32(6);
			Compile_SHR_32();
			Compile_POP_R32(0x24);
			
			if (Opcode[0]==0x75 || (Opcode[0]==0xF && Opcode[1]==0x85)) //NOT
				Compile_NOTBit();

			Compile_JCCIn();


			*dwNext=VMOpcodeBase+UsedSize;

		}
		else if (Opcode[0]==0x72 || Opcode[0]==0x73 || (Opcode[0]==0xF && (Opcode[1]==0x82 ||Opcode[1]==0x83 )) ) 
		{
			Compile_PUSH_R32(0x20);
			Compile_PUSH_Imm32(1);
			Compile_AND_32();
			Compile_POP_R32(0x24);
			
			if (Opcode[0]==0x73 || (Opcode[0]==0xF && Opcode[1]==0x83)) 
				Compile_NOTBit();

			Compile_JCCIn();


			*dwNext=VMOpcodeBase+UsedSize;

		}
		else if (Opcode[0]==0x7E || Opcode[0]==0x7F || (Opcode[0]==0xF && (Opcode[1]==0x8E ||Opcode[1]==0x8F )) ) 
		{
			Compile_PUSH_R32(0x20);
			Compile_PUSH_Imm32(0x40);
			Compile_AND_32();
			Compile_POP_R32(0x24);
			Compile_PUSH_Imm32(6);
			Compile_SHR_32();
			Compile_POP_R32(0x24); 
			Compile_NOTBit();

			Compile_PUSH_R32(0x20);
			Compile_PUSH_Imm32(0x800);
			Compile_AND_32();
			Compile_POP_R32(0x24);
			Compile_PUSH_Imm32(11);
			Compile_SHR_32();
			Compile_POP_R32(0x24); 

			Compile_PUSH_R32(0x20);
			Compile_PUSH_Imm32(0x80);
			Compile_AND_32();
			Compile_POP_R32(0x24);
			Compile_PUSH_Imm32(7);
			Compile_SHR_32();
			Compile_POP_R32(0x24);

			Compile_CMP_32();
			Compile_PUSH_Imm32(0x40);
			Compile_AND_32();
			Compile_POP_R32(0x24);
			Compile_PUSH_Imm32(6);
			Compile_SHR_32();
			Compile_POP_R32(0x24); 
			

			Compile_AND_32();
			Compile_POP_R32(0x24);



			
			if (Opcode[0]==0x7E || (Opcode[0]==0xF && Opcode[1]==0x8E))
				Compile_NOTBit();

			Compile_JCCIn();


			*dwNext=VMOpcodeBase+UsedSize;

		}
		else if (Opcode[0]==0x7C || Opcode[0]==0x7D || (Opcode[0]==0xF && (Opcode[1]==0x8C ||Opcode[1]==0x8D )) ) 
		{

			Compile_PUSH_R32(0x20);
			Compile_PUSH_Imm32(0x800);
			Compile_AND_32();
			Compile_POP_R32(0x24);
			Compile_PUSH_Imm32(11);
			Compile_SHR_32();
			Compile_POP_R32(0x24);

			Compile_PUSH_R32(0x20);
			Compile_PUSH_Imm32(0x80);
			Compile_AND_32();
			Compile_POP_R32(0x24);
			Compile_PUSH_Imm32(7);
			Compile_SHR_32();
			Compile_POP_R32(0x24); 

			Compile_CMP_32();
			Compile_PUSH_Imm32(0x40);
			Compile_AND_32();
			Compile_POP_R32(0x24);
			Compile_PUSH_Imm32(6);
			Compile_SHR_32();
			Compile_POP_R32(0x24); 
			


			if (Opcode[0]==0x7C || (Opcode[0]==0xF && Opcode[1]==0x8C))
				Compile_NOTBit();

			Compile_JCCIn();


			*dwNext=VMOpcodeBase+UsedSize;


		}

	}
	else
	{
		
	
	}


}
void CVMCompiler::ProcessPUSHR32(BYTE iOpcode)
{
     // Generate VM Opcode of "Push Register"
	if (iOpcode!=0x54)
		Compile_PUSH_R32(0x1C - (iOpcode-0x50)*4);
	else
	{
		OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_PUSHESP);
		UsedSize++;
		Compile_PUSH_Imm32(4);
		Compile_ADD_32();
		Compile_POP_R32(0x30);
		

	}

}
void CVMCompiler::ProcessPOPR32(BYTE iOpcode)
{
     // Generate VM Opcode of "POP Register"
	if (iOpcode!=0x5C)
		Compile_POP_R32(0x1C - (iOpcode-0x58)*4);
	else
	{
		OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_POPESP);
		UsedSize++;

	}

}
void CVMCompiler::ProcessMOVR32R32(LPBYTE iOpcode)
{
     // Generate VM Opcode of "Mov Register/Register"
	int rDEST;
	int rSRC;
	if (iOpcode[0]==0x89)
	{
		rDEST=(iOpcode[1]-0xC0)%8;
		rSRC=(iOpcode[1]-0xC0)/8;
	}
	else
	{
		rSRC=(iOpcode[1]-0xC0)%8;
		rDEST=(iOpcode[1]-0xC0)/8;

	}

	

	ProcessPUSHR32(0x50+rSRC);
	ProcessPOPR32(0x58+rDEST);
}
void CVMCompiler::ProcessADDR32_Imm(LPBYTE iOpcode)
{
     // Generate VM Opcode of "Add Register/Register"	
	if (iOpcode[0]!=0x81 && iOpcode[0]!=0x83)
		return ;
	DWORD dwImm=*(LPDWORD)(&iOpcode[2]);

	if (iOpcode[0]==0x83)
	{
		__asm 
		{
			push eax
				mov eax,dwImm
				movsx eax,al
				mov dwImm,eax
			pop eax

		}

	}

	int rDEST=iOpcode[1]-0xC0;

	ProcessPUSHR32(0x50 + rDEST);
	Compile_PUSH_Imm32(dwImm);
	Compile_ADD_32();
	Compile_POP_R32(0x20);
	ProcessPOPR32(0x58+rDEST);



}
void CVMCompiler::ProcessSUBR32_Imm(LPBYTE iOpcode)
{
     // Generate VM Opcode of "SUB"	
	if (iOpcode[0]!=0x81 && iOpcode[0]!=0x83)
		return ;
	long dwImm=*(LPLONG)(&iOpcode[2]);
	
	if (iOpcode[0]==0x83)
	{
		__asm 
		{
			push eax
				mov eax,dwImm
				movsx eax,al
				mov dwImm,eax
			pop eax

		}

	}
	dwImm=dwImm*-1;
	int rDEST=iOpcode[1]-0xE8;

	ProcessPUSHR32(0x50 + rDEST);
	Compile_PUSH_Imm32(dwImm);
	Compile_ADD_32();
	Compile_POP_R32(0x20);
	ProcessPOPR32(0x58+rDEST);




}
void CVMCompiler::ProcessMOV_MemR32Imm8_R32(LPBYTE iOpcode)
{
	
     // Generate VM Opcode of "Mov Register/Memory"
	DWORD dwImm;
	int rSRC;
	int rDEST;
	if ((iOpcode[1]-0x44)%8==4)
	{
		dwImm=*(LPDWORD)(&iOpcode[2]);
		__asm
		{
			push eax
				mov eax,dwImm
				movsx eax,al
				mov dwImm,eax
			pop eax

		}
		rSRC=(iOpcode[1]-0x40)/8;
		rDEST=(iOpcode[1]-0x40)%8;
	}
	else
	{
		dwImm=*(LPDWORD)(&iOpcode[3]);
		__asm
		{
			push eax
				mov eax,dwImm
				movsx eax,al
				mov dwImm,eax
			pop eax

		}
		rSRC=(iOpcode[1]-0x44)/8;;

		if (iOpcode[2]>=0x20 && iOpcode[2]<0x28)
			rDEST=0x20;
		else if (iOpcode[2]>=0x60 && iOpcode[2]<0x68)
			rDEST=0x60;
		else if (iOpcode[2]>=0xE0 && iOpcode[2]<0xE8)
			rDEST=0xE0;

		rDEST=(iOpcode[2]-rDEST)%8;
	}
	ProcessPUSHR32(0x50 + rDEST);
	Compile_PUSH_Imm32(dwImm);
	Compile_ADD_32();
	Compile_POP_R32(0x30);
	ProcessPUSHR32(0x50 + rSRC);
	if (rSRC==4)//SRC ESP
	{
		Compile_PUSH_Imm32(4);
		Compile_ADD_32();
		Compile_POP_R32(0x30);
	}

	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_SetDWORDDS);
	UsedSize++;
	
}
void CVMCompiler::ProcessMOV_MemR32Imm32_R32(LPBYTE iOpcode)
{
	
     // Generate VM Opcode of "Mov Memory/Register"
	DWORD dwImm;
	int rSRC;
	int rDEST;
	if ((iOpcode[1]-0x80)%8!=4)
	{
		dwImm=*(LPDWORD)(&iOpcode[2]);

		rSRC=(iOpcode[1]-0x80)/8;
		rDEST=(iOpcode[1]-0x80)%8;
	}
	else
	{
		dwImm=*(LPDWORD)(&iOpcode[3]);
		rSRC=(iOpcode[1]-0x84)/8;

		if (iOpcode[2]>=0x20 && iOpcode[2]<0x28)
			rDEST=0x20;
		else if (iOpcode[2]>=0x60 && iOpcode[2]<0x68)
			rDEST=0x60;
		else if (iOpcode[2]>=0xE0 && iOpcode[2]<0xE8)
			rDEST=0xE0;

		rDEST=(iOpcode[2]-rDEST)%8;
	}
	ProcessPUSHR32(0x50 + rDEST);
	Compile_PUSH_Imm32(dwImm);
	Compile_ADD_32();
	Compile_POP_R32(0x30);
	ProcessPUSHR32(0x50 + rSRC);
	if (rSRC==4)//SRC ESP
	{
		Compile_PUSH_Imm32(4);
		Compile_ADD_32();
		Compile_POP_R32(0x30);
	}

	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_SetDWORDDS);
	UsedSize++;
	
}
void CVMCompiler::ProcessMOV_MemR32Imm32_Imm32(LPBYTE iOpcode)
{
	
     // Generate VM Opcode of "Mov Memory/Memory"
	DWORD dwSRCImm;
	DWORD dwDestImm;

	int rDEST;
	if ((iOpcode[1]-0x80)%8!=4)
	{
		dwDestImm=*(LPDWORD)(&iOpcode[2]);
		dwSRCImm=*(LPDWORD)(&iOpcode[6]);

		rDEST=(iOpcode[1]-0x80)%8;
	}
	else
	{
		dwDestImm=*(LPDWORD)(&iOpcode[3]);
		dwSRCImm=*(LPDWORD)(&iOpcode[7]);
		

		if (iOpcode[2]>=0x20 && iOpcode[2]<0x28)
			rDEST=0x20;
		else if (iOpcode[2]>=0x60 && iOpcode[2]<0x68)
			rDEST=0x60;
		else if (iOpcode[2]>=0xE0 && iOpcode[2]<0xE8)
			rDEST=0xE0;

		rDEST=(iOpcode[2]-rDEST)%8;
	}
	ProcessPUSHR32(0x50 + rDEST);
	Compile_PUSH_Imm32(dwDestImm);
	Compile_ADD_32();
	Compile_POP_R32(0x30);
	Compile_PUSH_Imm32(dwSRCImm);

	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_SetDWORDDS);
	UsedSize++;
	
}
void CVMCompiler::ProcessMOV_R32_MemR32Imm32(LPBYTE iOpcode)
{
	
     // Generate VM Opcode of "Mov Mem32/Register32"
	DWORD dwImm;
	int rSRC;
	int rDEST;
	if ((iOpcode[1]-0x80)%8!=4)
	{
		dwImm=*(LPDWORD)(&iOpcode[2]);

		rDEST=(iOpcode[1]-0x80)/8;
		rSRC=(iOpcode[1]-0x80)%8;
	}
	else
	{
		dwImm=*(LPDWORD)(&iOpcode[3]);
		rDEST=(iOpcode[1]-0x84)/8;

		if (iOpcode[2]>=0x20 && iOpcode[2]<0x28)
			rSRC=0x20;
		else if (iOpcode[2]>=0x60 && iOpcode[2]<0x68)
			rSRC=0x60;
		else if (iOpcode[2]>=0xE0 && iOpcode[2]<0xE8)
			rSRC=0xE0;

		rSRC=(iOpcode[2]-rSRC)%8;
	}
	ProcessPUSHR32(0x50 + rSRC);
	Compile_PUSH_Imm32(dwImm);
	Compile_ADD_32();
	Compile_POP_R32(0x30);
	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_GetDWORDDS);
	UsedSize++;
	ProcessPOPR32(0x58+rDEST);
	
}
void CVMCompiler::ProcessMOV_R32_MemR32Imm8(LPBYTE iOpcode)
{
	
     // Generate VM Opcode of "Memory / DWORD Memory"
	DWORD dwImm;
	int rSRC;
	int rDEST;
	if ((iOpcode[1]-0x40)%8!=4)
	{
		dwImm=*(LPDWORD)(&iOpcode[2]);

		rDEST=(iOpcode[1]-0x40)/8;
		rSRC=(iOpcode[1]-0x40)%8;
	}
	else
	{
		dwImm=*(LPDWORD)(&iOpcode[3]);
		rDEST=(iOpcode[1]-0x44)/8;

		if (iOpcode[2]>=0x20 && iOpcode[2]<0x28)
			rSRC=0x20;
		else if (iOpcode[2]>=0x60 && iOpcode[2]<0x68)
			rSRC=0x60;
		else if (iOpcode[2]>=0xE0 && iOpcode[2]<0xE8)
			rSRC=0xE0;

		rSRC=(iOpcode[2]-rSRC)%8;
	}
	__asm
	{
		push eax
			mov eax,dwImm
			movsx eax,al
			mov dwImm,eax
		pop eax

	}
	ProcessPUSHR32(0x50 + rSRC);
	Compile_PUSH_Imm32(dwImm);
	Compile_ADD_32();
	Compile_POP_R32(0x30);
	OpcodeTable[UsedSize]=iVMHandler.GetHandlerIndex(VM_GetDWORDDS);
	UsedSize++;
	ProcessPOPR32(0x58+rDEST);
	
}
void CVMCompiler::ProcessLEA_R32_MemR32Imm32(LPBYTE iOpcode)
{
	
     // Generate VM Opcode of "Mov Mem/16 bit Register"
	DWORD dwImm;
	int rSRC;
	int rDEST;
	if ((iOpcode[1]-0x80)%8!=4)
	{
		dwImm=*(LPDWORD)(&iOpcode[2]);

		rDEST=(iOpcode[1]-0x80)/8;
		rSRC=(iOpcode[1]-0x80)%8;
	}
	else
	{
		dwImm=*(LPDWORD)(&iOpcode[3]);
		rDEST=(iOpcode[1]-0x84)/8;

		if (iOpcode[2]>=0x20 && iOpcode[2]<0x28)
			rSRC=0x20;
		else if (iOpcode[2]>=0x60 && iOpcode[2]<0x68)
			rSRC=0x60;
		else if (iOpcode[2]>=0xE0 && iOpcode[2]<0xE8)
			rSRC=0xE0;

		rSRC=(iOpcode[2]-rSRC)%8;
	}
	ProcessPUSHR32(0x50 + rSRC);
	Compile_PUSH_Imm32(dwImm);
	Compile_ADD_32();
	Compile_POP_R32(0x30);
	ProcessPOPR32(0x58 + rDEST);
	
}
void CVMCompiler::ProcessXORR32_R32(LPBYTE iOpcode)
{
     // Generate VM Opcode of "XOR"
	if (iOpcode[0]!=0x33 && iOpcode[0]!=0x31 )
		return;

	int rSRC=(iOpcode[1]-0xC0)/8;
	int rDEST=(iOpcode[1]-0xC0)%8;

	ProcessPUSHR32(0x50+rSRC);
	ProcessPUSHR32(0x50+rDEST);
	Compile_NOT_32();
	Compile_AND_32();
	Compile_POP_R32(0x30);

	ProcessPUSHR32(0x50+rDEST);
	ProcessPUSHR32(0x50+rSRC);
	Compile_NOT_32();
	Compile_AND_32();
	Compile_POP_R32(0x30);

	Compile_ADD_32();
	Compile_POP_R32(0x30);

	ProcessPOPR32(0x58 + rDEST);

}
void CVMCompiler::ProcessMOVR32_Imm(LPBYTE iOpcode)
{
     // Generate VM Opcode of "Mov Register16/Register16" (16 means 16 bit)
	if (iOpcode[0]<0xB8 || iOpcode[0]>0xBF)
		return;

	DWORD dwImm=*(LPDWORD)(&iOpcode[1]);

	Compile_PUSH_Imm32(dwImm);
	ProcessPOPR32(0x58 + iOpcode[0] - 0xB8);
}
void CVMCompiler::ProcessMOVR32_MemImm(LPBYTE iOpcode)
{
	// Generate VM Opcode of "Mov Register16/memory16" (16 means 16 bit)
	DWORD dwImm;
	int rDEST;
	if (iOpcode[0]==0xA1)
	{
		dwImm=*(LPDWORD)(&iOpcode[1]);
		rDEST=0;
	}
	else
	{
		dwImm=*(LPDWORD)(&iOpcode[2]);
		rDEST=(iOpcode[1]-0xD)/8 + 1;
	}
	
	Compile_PUSH_Imm32(dwImm);
	Compile_GetDWORDDS();
	ProcessPOPR32(0x58 + rDEST);



}
void CVMCompiler::ProcessMOVMemImm_R32(LPBYTE iOpcode)
{
	DWORD dwImm;
	int rSRC;
	if (iOpcode[0]==0xA3)
	{
		dwImm=*(LPDWORD)(&iOpcode[1]);
		rSRC=0;
	}
	else
	{
		dwImm=*(LPDWORD)(&iOpcode[2]);
		rSRC=(iOpcode[1]-0xD)/8 + 1;
	}
	
	Compile_PUSH_Imm32(dwImm);
	ProcessPUSHR32(0x50+rSRC);
	Compile_SetDWORDDS();
	
}
void CVMCompiler::ProcessANDR32_Imm(LPBYTE iOpcode)
{
     // Generate VM Opcode of "AND Register"
	DWORD dwImm;
	int rDEST;
	if (iOpcode[0]==0x25)
	{
		dwImm=*(LPDWORD)(&iOpcode[1]);
		rDEST=0;
	}
	else
	{
		dwImm=*(LPDWORD)(&iOpcode[2]);
		rDEST=(iOpcode[1]-0xE0);
	}
	
	ProcessPUSHR32(0x50 + rDEST);
	Compile_PUSH_Imm32(dwImm);
	Compile_AND_32();
	Compile_POP_R32(0x30);
	ProcessPOPR32(0x58 + rDEST);
	
}
void CVMCompiler::ProcessORR32_Imm(LPBYTE iOpcode)
{
     // Generate VM Opcode of "OR Register/memory"
	DWORD dwImm;
	int rDEST;
	if (iOpcode[0]==0xD)
	{
		dwImm=*(LPDWORD)(&iOpcode[1]);
		rDEST=0;
	}
	else
	{
		dwImm=*(LPDWORD)(&iOpcode[2]);
		if (iOpcode[0]==0x83) 
		{
			__asm
			{
				push eax
					mov eax,dwImm
					movsx eax,al
					mov dwImm,eax
				pop eax

			}

		}
		rDEST=(iOpcode[1]-0xC8);
	}
	
	ProcessPUSHR32(0x50 + rDEST);
	Compile_NOT_32();
	Compile_PUSH_Imm32(dwImm^0xFFFFFFFF);
	Compile_AND_32();
	Compile_POP_R32(0x30);
	Compile_NOT_32();
	Compile_PUSH_Imm32(0xFFFFFFFF);
	Compile_AND_32();
	Compile_POP_R32(0x20); 
	ProcessPOPR32(0x58 + rDEST);
	
}
void CVMCompiler::ProcessSHLR32_Imm(LPBYTE iOpcode)
{
     // Generate VM Opcode of "SHL Register - memory"
	int rDEST=iOpcode[1]-0xE0;
	DWORD dwImm;
	__asm
	{
		push eax
			mov eax,iOpcode
			movzx eax,byte ptr ds:[eax+2]
			mov dwImm,eax
		pop eax
	}

	ProcessPUSHR32(0x50 + rDEST);
	Compile_PUSH_Imm32(dwImm);
	Compile_SHL_32();
	Compile_POP_R32(0x30);
	ProcessPOPR32(0x58 + rDEST);

}
void CVMCompiler::ProcessSHRR32_Imm(LPBYTE iOpcode)
{
     // Generate VM Opcode of "SHR Register  -   memory"
	int rDEST=iOpcode[1]-0xE8;
	DWORD dwImm;
	__asm
	{
		push eax
			mov eax,iOpcode
			movzx eax,byte ptr ds:[eax+2]
			mov dwImm,eax
		pop eax
	}

	ProcessPUSHR32(0x50 + rDEST);
	Compile_PUSH_Imm32(dwImm);
	Compile_SHR_32();
	Compile_POP_R32(0x30);
	ProcessPOPR32(0x58 + rDEST);

}
void CVMCompiler::ProcessNOTR32(LPBYTE iOpcode)
{
     // Generate VM Opcode of "NOT Register"
	int rDEST=iOpcode[1]-0xD0;
	ProcessPUSHR32(0x50+rDEST);
	Compile_NOT_32();
	ProcessPOPR32(0x58+rDEST);

}
void CVMCompiler::ProcessINC_DECR32(LPBYTE iOpcode)
{
	// Generate VM Opcode of "DEC Register"
	int rDEST;
	int dwAdd;

	if (iOpcode[0]<0x48)
	{
		dwAdd=1;
		rDEST=iOpcode[0]-0x40;
	}
	else
	{
		dwAdd=0xFFFFFFFF;
		rDEST=iOpcode[0]-0x48;
	}

	ProcessPUSHR32(0x50+rDEST);
	Compile_PUSH_Imm32(dwAdd);
	Compile_ADD_32();
	Compile_POP_R32(0x20);
	ProcessPOPR32(0x58+rDEST);

}
void CVMCompiler::ProcessCMPR32_Imm(LPBYTE iOpcode)
{
     // Generate VM Opcode of "CMP Register - Memory"	
	if (iOpcode[0]!=0x81 && iOpcode[0]!=0x83 && iOpcode[0]!=0x3D)
		return ;
	DWORD dwImm;
	int rDEST=0;
	if (iOpcode[0]!=0x3D)
	{

		rDEST=iOpcode[1]-0xF8;
		dwImm=*(LPDWORD)(&iOpcode[2]);

		if (iOpcode[0]==0x83) 
		{
			__asm 
			{
				push eax
					mov eax,dwImm
					movsx eax,al
					mov dwImm,eax
				pop eax

			}
		}

	}
	else
		dwImm=*(LPDWORD)(&iOpcode[1]);
	
	
    // Finalize ...
	ProcessPUSHR32(0x50 + rDEST);
	Compile_NOT_32();
	Compile_PUSH_Imm32(dwImm);
	Compile_ADD_32();
	Compile_POP_R32(0x30);
	Compile_PUSH_R32(0x30);
	Compile_PUSH_Imm32(0x815);
	Compile_AND_32();
	Compile_POP_R32(0x34);
	Compile_POP_R32(0x38);
	Compile_NOT_32();
	Compile_POP_R32(0x34);
	Compile_PUSH_R32(0x34);
	Compile_PUSH_R32(0x34);
	Compile_AND_32();
	Compile_PUSH_Imm32(0x815);
	Compile_NOT_32();
	Compile_AND_32();
	Compile_POP_R32(0x30);
	Compile_PUSH_R32(0x38);

	Compile_ADD_32();
	Compile_POP_R32(0x20);
	Compile_POP_R32(0x20);
	Compile_POP_R32(0x30);






}