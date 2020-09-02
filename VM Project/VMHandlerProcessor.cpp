#include "stdafx.h"
#include "VMHandlerProcessor.h"


CVMHandlerProcessor::CVMHandlerProcessor(void)
{
	HandlerCount=0;

}


CVMHandlerProcessor::~CVMHandlerProcessor(void)
{
}
unsigned CVMHandlerProcessor::GetHandlerIndex(VMHandlerCodes iType)
{
    // Main VM Handler 
	unsigned int Result=0xFFFFFFFF;

	for (unsigned int i=0;i<HandlerCount;i++)
		if (HandlerTable[i]==iType)
			Result=i;
	
	if (Result==0xFFFFFFFF)
	{
		DWORD iHandlerAddr=0;
		switch (iType)
		{
		case VM_Start:
			__asm push VMH_Start
			__asm pop iHandlerAddr;
			break;

		case VM_Exit:
			__asm push VMH_Exit
			__asm pop iHandlerAddr;
			break;
			
		case VM_X86:
			__asm push VMH_X86
			__asm pop iHandlerAddr;
			break;
		case VM_PUSHImmDW:
			__asm push VMH_PUSHImmDWORD
			__asm pop iHandlerAddr;
			break;
		case VM_JCCIn:
			__asm push VMH_JCCIn;
			__asm pop iHandlerAddr;
			break;
		case VM_CALL:
			__asm push VMH_CALL
			__asm pop iHandlerAddr;
			break;
		case VM_PUSHR32:
			__asm push VMH_PUSHR32
			__asm pop iHandlerAddr;
			break;
		case VM_POPR32:
			__asm push VMH_POPR32
			__asm pop iHandlerAddr;
			break;
		case VM_AND32:
			__asm push VMH_AND32
			__asm pop iHandlerAddr;
			break;
		case VM_NOT32:
			__asm push VMH_NOT32
			__asm pop iHandlerAddr;
			break;
		case VM_SHR32:
			__asm push VMH_SHR32;
			__asm pop iHandlerAddr;
			break;
		case VM_SHL32:
			__asm push VMH_SHL32;
			__asm pop iHandlerAddr;
			break;
		case VM_ADD32:
			__asm push VMH_ADD32;
			__asm pop iHandlerAddr;
			break;
		case VM_PUSHESP:
			__asm push VMH_PUSHESP;
			__asm pop iHandlerAddr;
			break;
		case VM_POPESP:
			__asm push VMH_POPESP;
			__asm pop iHandlerAddr;
			break;
		case VM_GetDWORDDS:
			__asm push VMH_GetDWORDDS;
			__asm pop iHandlerAddr;
			break;
		case VM_SetDWORDDS:
			__asm push VMH_SetDWORDDS;
			__asm pop iHandlerAddr;
			break;

		};
		HandlerCurrentTable[HandlerCount]=GetMainAddress((LPBYTE)iHandlerAddr);
		HandlerTable[HandlerCount]=iType;
		Result=HandlerCount;
		HandlerCount++;

	}

	return Result;

}


DWORD CVMHandlerProcessor::BuildVMEngine(DWORD VBase,LPBYTE MemBase)
{
     // Add VM Opcodes and build related vm handlers...
	DWORD UsedSize=0;
	DWORD tmp;
	DWORD iVA=VBase;
	LPBYTE iMem=MemBase;

	LPDWORD HandlerTable=(LPDWORD)iMem;
	dwVMHandlerTableAddress=iVA;

	iVA+=HandlerCount*4;
	iMem+=HandlerCount*4;

	
	for (unsigned int i=0;i<HandlerCount;i++)
	{
		HandlerTable[i]=iVA;
		CopyMemory(iMem,(LPBYTE)HandlerCurrentTable[i],GetHandlerSize(LPBYTE(HandlerCurrentTable[i])));
		iVA+=GetHandlerSize(LPBYTE(HandlerCurrentTable[i]));
		iMem+=GetHandlerSize(LPBYTE(HandlerCurrentTable[i]));
		UsedSize+=4;
		UsedSize+=GetHandlerSize(LPBYTE(HandlerCurrentTable[i]));
	}

	
	dwVMEntryAddress=iVA;
	__asm push VMH_Start
	__asm pop tmp
	tmp=GetMainAddress((LPBYTE)tmp);
	CopyMemory(iMem,(LPVOID)tmp,GetHandlerSize((LPBYTE)tmp));
	iVA+=GetHandlerSize((LPBYTE)tmp);
	iMem+=GetHandlerSize((LPBYTE)tmp);
	UsedSize+=GetHandlerSize((LPBYTE)tmp);

	__asm push VMH_Dispatcher
	__asm pop tmp
	tmp=GetMainAddress((LPBYTE)tmp);
	CopyMemory(iMem,(LPVOID)tmp,GetHandlerSize((LPBYTE)tmp));
	UsedSize+=GetHandlerSize((LPBYTE)tmp);
	
	{
		LPBYTE Base=iMem;
		if (Base[0]==0xE9)
			Base=(Base+*((LPDWORD)(Base+1))+5);
		while (*((LPDWORD)Base)!=0x12345678)
			Base++;
		*((LPDWORD)Base)=dwVMHandlerTableAddress;

	}
	iVA+=GetHandlerSize((LPBYTE)tmp);
	iMem+=GetHandlerSize((LPBYTE)tmp);
	
	

	return UsedSize;


}

DWORD CVMHandlerProcessor::GetHandlerSize(LPBYTE HandlerMem)
{
	LPBYTE Current=HandlerMem;
	if (Current[0]==0xE9)
	{
		Current=(Current+*((LPDWORD)(Current+1))+5);
		HandlerMem=Current;
	}
	while (Current[0]!=0x68 || *(LPDWORD(&Current[1]))!=0xDEADC0DE)
		Current++;

	return Current-HandlerMem;
}

DWORD CVMHandlerProcessor::GetMainAddress(LPBYTE Current)
{
	if (Current[0]==0xE9)
		Current=(Current+*((LPDWORD)(Current+1))+5);
	return (DWORD)Current;
}

void __declspec(naked) CVMHandlerProcessor::VMH_Start()
{
     // Start of VM init...
	__asm
	{
		LEA ESP,[ESP-0x11C]
		PUSHFD
		PUSHAD
		MOV EBP,ESP
		ADD EBP,0x140
		MOV EDI,ESP
		SUB ESP,0x20
		MOV ESI,DWORD PTR SS:[EBP]
		ADD EBP,4

		//---------------
		PUSH 0xDEADC0DE
	}
}

void __declspec(naked) CVMHandlerProcessor::VMH_Exit()
{
     // Exit from VM Handler and continue normal execution...
	__asm
	{
		NOP
		NOP


		//---------------
		PUSH 0xDEADC0DE
	}
}

void __declspec(naked)CVMHandlerProcessor::VMH_X86()
{
     // VM Handler of "Call Register" Opcode
	__asm
	{
		//-----------Write CallBack
		CALL HERE0
HERE0:
		POP EAX
		ADD EAX,0x8B
		POP DWORD PTR DS:[EAX]
		//-------Restore VM Context
		PUSH ESI
		PUSH EDI
		
		
		CALL HERE1
		//------------------

HERE1:
		POP EDI
		ADD EDI,0x48
		//------------
		PUSHAD
		MOV EAX,0x90909090
		MOV ECX,4
		REP STOS DWORD PTR ES:[EDI]
		POPAD
		//-------------------------

		MOVZX ECX,BYTE PTR DS:[ESI]
		ADD ESI,1
		REP MOVS BYTE PTR ES:[EDI],BYTE PTR DS:[ESI]
		POP EDI
		POP ESI
		CALL HERE2
HERE2:
		POP EBX
		ADD EBX,0x37
		MOVZX ECX,[ESI]
		ADD ESI,ECX
		INC ESI
		MOV [EBX],ESI


		MOV EAX,[EDI+0x1C]
		MOV EBX,[EDI+0x10]
		MOV ECX,[EDI+0x18]
		MOV EDX,[EDI+0x14]
		MOV ESP,EBP
		MOV EBP,[EDI+0x08]
		MOV ESI,[EDI+0x04]
		PUSH [EDI+0x20]
		POPFD
		MOV EDI,[EDI+0x0]
		//---------------------------x86 Cave

		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		//-----------------------Reset the VM
		PUSH 0X1111111
		LEA ESP,[ESP-0x11C]
		PUSHFD
		PUSHAD
		MOV EBP,ESP
		ADD EBP,0x140
		MOV EDI,ESP
		SUB ESP,0x20
		MOV ESI,DWORD PTR SS:[EBP]
		ADD EBP,4
		

		//---------------
		PUSH 0x11112222
		RET
		PUSH 0xDEADC0DE
	}

}


void __declspec(naked) CVMHandlerProcessor::VMH_CALL()
{
     // VM Handler of all "CALL Address" Opcode
	__asm
	{
		
		__asm
	{
		//-----------Write CallBack
		CALL HERE0
HERE0:
		POP EAX
		ADD EAX,0x6F
		POP DWORD PTR DS:[EAX]
		//-------Restore VM Context
		
		
		CALL HERE1
		//------------------

HERE1:
		POP EBX
		ADD EBX,0x3B
		//-------------------------
		MOV BYTE PTR DS:[EBX],0xE8
		MOV ECX,[ESI]
		SUB ECX,EBX
		SUB ECX,0x5
		MOV DWORD PTR DS:[EBX+1],ECX
		CALL HERE2
HERE2:
		POP EBX
		ADD EBX,0x29
		ADD ESI,4
		MOV [EBX],ESI


		MOV EAX,[EDI+0x1C]
		MOV EBX,[EDI+0x10]
		MOV ECX,[EDI+0x18]
		MOV EDX,[EDI+0x14]
		MOV ESP,EBP
		MOV EBP,[EDI+0x08]
		MOV ESI,[EDI+0x04]
		PUSH [EDI+0x20]
		POPFD
		MOV EDI,[EDI+0x0]
		//---------------------------x86 Cave

		NOP
		NOP
		NOP
		NOP
		NOP
		//-----------------------Reset the VM
		PUSH 0X1111111
		LEA ESP,[ESP-0x11C]
		PUSHFD
		PUSHAD
		MOV EBP,ESP
		ADD EBP,0x140
		MOV EDI,ESP
		SUB ESP,0x20
		MOV ESI,DWORD PTR SS:[EBP]
		ADD EBP,4
		

		//---------------
		PUSH 0x11112222
		RET
		PUSH 0xDEADC0DE
	}
		//---------------
		PUSH 0xDEADC0DE
	}
}


void __declspec(naked) CVMHandlerProcessor::VMH_Dispatcher()
{
     // VM Dispatcher , this code reset vm context and go next vm handler
	 
	__asm
	{
Entry:		
		LEA EAX,[EDI+0x40]
		CMP EBP,EAX
		Ja DoDispatch
		//---------------MOVE VM_Context
		PUSH ESI
		MOV ESI,EDI
		LEA EDI,[EBP-0x100]
		MOV ECX,0x100
		CLD
		REP MOVS BYTE PTR ES:[EDI],BYTE PTR DS:[ESI]
		POP ESI
		LEA EDI,[EBP-0x100]

DoDispatch:
		MOVZX EAX,BYTE PTR DS:[ESI]
		INC ESI
		CALL DWORD PTR DS:[EAX*4+0x12345678]
		JMP Entry


	    //---------------
		PUSH 0xDEADC0DE
	}
}
void __declspec(naked) CVMHandlerProcessor::VMH_PUSHImmDWORD()
{
     // VM Handler of "Push DWORD Data" Opcode
	__asm
	{
		MOV EAX, DWORD PTR DS:[ESI]
		ADD ESI,4
		SUB EBP,4
		MOV [EBP],EAX
		RET
	    //---------------
		PUSH 0xDEADC0DE
	}
}
void __declspec(naked) CVMHandlerProcessor::VMH_JCCIn()
{
     // VM Handler of "JCC" Opcode
	__asm
	{
		MOVZX EAX,BYTE PTR DS:[EBP]
		ADD EBP,4
		MOV ESI,[EBP+4*EAX]
		ADD EBP,8
		RET
	    //---------------
		PUSH 0xDEADC0DE
	}
}
void __declspec(naked) CVMHandlerProcessor::VMH_PUSHR32()
{
     // VM Handler of "Push Register/Address" Opcode
	__asm
	{
		MOVZX EAX,BYTE PTR DS:[ESI]
		ADD ESI,1
		ADD EAX,EDI
		MOV EAX,[EAX]
		SUB EBP,4
		MOV [EBP],EAX
		RET
	    //---------------
		PUSH 0xDEADC0DE
	}


}
void __declspec(naked) CVMHandlerProcessor::VMH_POPR32()
{
     // VM Handler of "POP Regsiter/Address" Opcode
	__asm
	{
		MOVZX EAX,BYTE PTR DS:[ESI]
		ADD ESI,1
		MOV EDX,[EBP]
		ADD EBP,4
		MOV [EAX+EDI],EDX
		RET
	    //---------------
		PUSH 0xDEADC0DE
	}


}
void __declspec(naked) CVMHandlerProcessor::VMH_AND32()
{
     // VM Handler of "AND" Opcode
	__asm
	{
		MOV EAX,[EBP]
		AND DWORD PTR DS:[EBP+4],EAX
		PUSHFD
		POP DWORD PTR DS:[EBP]
		RET
	    //---------------
		PUSH 0xDEADC0DE
	}


}
void __declspec(naked) CVMHandlerProcessor::VMH_NOT32()
{
     // VM Handler of "NOT" Opcode
	__asm
	{
		NOT DWORD PTR DS:[EBP]
		RET
	    //---------------
		PUSH 0xDEADC0DE
	}


}
void __declspec(naked) CVMHandlerProcessor::VMH_SHR32()
{
     // VM Handler of "SHR" Opcode
	__asm
	{
		MOV EAX,[EBP+4]
		MOVZX ECX,[EBP]
		SHR EAX,CL
		MOV [EBP+4],EAX
		PUSHFD
		POP DWORD PTR DS:[EBP]
		RET
	    //---------------
		PUSH 0xDEADC0DE
	}


}
void __declspec(naked) CVMHandlerProcessor::VMH_SHL32()
{
     // VM Handler of "SHL" Opcode
	__asm
	{
		MOV EAX,[EBP+4]
		MOVZX ECX,[EBP]
		SHL EAX,CL
		MOV [EBP+4],EAX
		PUSHFD
		POP DWORD PTR DS:[EBP]
		RET
	    //---------------
		PUSH 0xDEADC0DE
	}


}
void __declspec(naked) CVMHandlerProcessor::VMH_ADD32()
{
     // VM Handler of "ADD" Opcode
	__asm
	{
		MOV EAX,[EBP]
		ADD [EBP+4],EAX
		PUSHFD
		POP DWORD PTR DS:[EBP]
		RET
	    //---------------
		PUSH 0xDEADC0DE
	}


}
void __declspec(naked) CVMHandlerProcessor::VMH_PUSHESP()
{
     // VM Handler of "Push esp" Opcode
	__asm
	{
		SUB EBP,4
		MOV [EBP],EBP
		RET

		//---------------
		PUSH 0xDEADC0DE
	}

}
void __declspec(naked) CVMHandlerProcessor::VMH_POPESP()
{
     // VM Handler of "Pop esp" Opcode
	__asm
	{
		MOV EBP,[EBP]
		RET

		//---------------
		PUSH 0xDEADC0DE
	}

}
void __declspec(naked) CVMHandlerProcessor::VMH_GetDWORDDS()
{
     // VM Handler of Any Opcode that get DWORD Data
	__asm
	{
		MOV EAX,[EBP]
		MOV EAX,DWORD PTR DS:[EAX]
		MOV [EBP],EAX
		RET

		//---------------
		PUSH 0xDEADC0DE
	}

}
void __declspec(naked) CVMHandlerProcessor::VMH_SetDWORDDS()
{
     // VM Handler of Any Opcode that set DWORD Data
	__asm
	{
		MOV EAX,[EBP]
		MOV ECX,[EBP+4]
		ADD EBP,8
		MOV DWORD PTR DS:[ECX],EAX
		RET

		//---------------
		PUSH 0xDEADC0DE
	}

}

