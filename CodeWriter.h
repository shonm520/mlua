#pragma once
#include "Value.h"
#include <string>
#include "Instruction.h"

typedef  Instruction_ Instruction;

struct Value_
{

};

/*class String : public Value
{
public:
	explicit String(const std::string& v)
		: value_(v)
	{
	}

	virtual std::string Name() const
	{
		return "string";
	}

// 	virtual std::size_t GetHash() const;
// 	virtual bool IsEqual(const Value *other) const;
// 	virtual void Mark();

	std::string Get()
	{
		return value_;
	}

	const std::string& Get() const
	{
		return value_;
	}

private:
	std::string value_;
};*/

// struct InstructionParam
// {
// 	enum InstructionParamType
// 	{
// 		InstructionParamType_None,
// 		InstructionParamType_Value,
// 		InstructionParamType_Name,
// 		InstructionParamType_Counter,
// 		InstructionParamType_CounterIndex,
// 		InstructionParamType_OpCodeIndex,
// 		InstructionParamType_ArrayIndex,
// 	};
// 
// 	InstructionParamType type;
// 	union
// 	{
// 		Value *value;
// 		String* name;
// 		int counter;
// 		int counter_index;
// 		int opcode_index;
// 		int array_index;
// 	} param;
// };

/*struct Instruction_
{
	enum OpCode
	{
		OpCode_Assign,
		OpCode_CleanStack,
		OpCode_GetLocalTable,
		OpCode_GetTable,
		OpCode_GetTableValue,
		OpCode_Push,
		OpCode_Pop,
		OpCode_GenerateClosure,
		OpCode_Ret,
		OpCode_GenerateArgTable,
		OpCode_MergeCounter,
		OpCode_ResetCounter,
		OpCode_DuplicateCounter,
		OpCode_Call,
		OpCode_AddLocalTable,
		OpCode_DelLocalTable,
		OpCode_AddGlobalTable,
		OpCode_DelGlobalTable,
		OpCode_Power,
		OpCode_Multiply,
		OpCode_Divide,
		OpCode_Mod,
		OpCode_Plus,
		OpCode_Minus,
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
};*/

class CodeWrite  {
public:
	CodeWrite();
	~CodeWrite();
	Instruction_* newInstruction();

	void* paramRW;        //可读写设置
	void* paramFunc;      //函数返回个数设置，有些地方需要返回值1到n个，有些地方不需

	void putInstructions(std::vector<Instruction*>&);

private:
	Instruction_* _instrucions;
	int num;
};
