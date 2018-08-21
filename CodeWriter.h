#pragma once
#include "Value.h"
#include <string>
#include "Instruction.h"


class InstructionValue;

class CodeWrite  {
public:
	CodeWrite();
	~CodeWrite();
	Instruction* newInstruction();

	void* paramRW;        //可读写设置
	void* paramFunc;      //函数返回个数设置，有些地方需要返回值1到n个，有些地方不需

	InstructionSet* fetchInstructionSet();

	InstructionValue* fetchInstructionVal();

private:
	InstructionSet* _insSet;
};
