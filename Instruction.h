#pragma once
#include <vector>
#include "Value.h"



struct Instruction;
class InstructionSet
{
public:
	enum Volum  {
		None,
		Small,
		Medium,
		Large
	};
	InstructionSet(Volum v);
	std::vector<Instruction*> toVtInstructions();

	Instruction* getInstructions(int& num) { num = _num; return _instructions; };
	void pushInstruct(Instruction* ins)  {
		_opcodes.push_back(ins);
	}
	Instruction* newInstruction();

	void clearInstructions();

private:
	std::vector<Instruction*> _opcodes;
	Instruction* _instructions;
	int _num;
};



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
// 	struct  Param{
// 		union {
// 			Value *value;
// 			String* name;
// 		};
// 		//int array_index;
// 		struct Counter  {
// 			int counter1;
// 			int counter2;
// 		};
// 	};

	//Param param;

	struct Counter  {
		int counter1;
		int counter2;
	};

	union {
		Value *value;
		String* name;
	};
	Counter counter;
};



struct Instruction
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
		OpCode_GenerateBlock,
		OpCode_Ret,
		OpCode_GenerateArgTable,
		OpCode_MergeCounter,
		OpCode_ResetCounter,
		OpCode_DuplicateCounter,
		OpCode_Call,
		OpCode_EnterClosure,
		OpCode_QuitClosure,
		OpCode_EnterBlock,
		OpCode_QuitBlock,
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

		OpCode_If,
		OpCode_NumericFor,
		OpCode_GenericFor,
		OpCode_Break,
		OpCode_TableDefine,
		OpCode_TableArrIndex,
		OpCode_TableMemAccess,
		OpCode_PassFunParam,
	};
	OpCode op_code;
	int type;
	InstructionParam param;
	Instruction();
};

