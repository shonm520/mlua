#pragma once
#include <vector>
#include "Value.h"


struct InstructionParam
{
	enum InstructionParamType
	{
		InstructionParamType_None,
		InstructionParamType_Value,
		InstructionParamType_Name,
		InstructionParamType_Counter,
		InstructionParamType_CounterIndex,
		InstructionParamType_OpCodeIndex,
		InstructionParamType_ArrayIndex,
	};

	InstructionParamType type;
	union
	{
		Value *value;
		String* name;
		int counter_index;
		int opcode_index;
		int array_index;
		struct Counter  {
			int counter1;
			int counter2;
		}counter;
	} param;
};



struct Instruction_
{
	enum OpCode
	{
		OpCode_UnKown,
		OpCode_Assign,
		OpCode_CleanStack,
		OpCode_GetLocalTable,
		OpCode_GetTable,
		OpCode_GetTableValue,
		OpCode_InitLocalVar,
		OpCode_GetLocalVar,
		OpCode_SetLocalVar,
		OpCode_Push,
		OpCode_Pop,
		OpCode_GenerateClosure,
		OpCode_Ret,
		OpCode_GenerateArgTable,
		OpCode_MergeCounter,
		OpCode_ResetCounter,
		OpCode_DuplicateCounter,
		OpCode_Call,
		OpCode_EnterClosure,
		OpCode_QuitClosure,
		OpCode_AddGlobalTable,
		OpCode_DelGlobalTable,
		OpCode_Plus,
		OpCode_Minus,
		OpCode_Multiply,
		OpCode_Divide,
		OpCode_Mod,
		OpCode_Power,
		OpCode_Concat,
		OpCode_Less,
		OpCode_Greater,
		OpCode_LessEqual,
		OpCode_GreaterEqual,
		OpCode_NotEqual,
		OpCode_Equal,
		OpCode_Not,
		OpCode_Length,
		OpCode_Negative,
		OpCode_JmpTrue,
		OpCode_JmpFalse,
		OpCode_JmpNil,
		OpCode_Jmp,
		OpCode_NewTable,
		OpCode_SetTableArrayValue,
	};
	OpCode op_code;
	InstructionParam param_a;
};

