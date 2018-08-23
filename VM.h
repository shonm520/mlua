#pragma once
#include <vector>
#include "Instruction.h"

class State;
class Stack;
class InstructionValue;

class VM
{
public:
	VM(State* state);
	~VM();

private:
	State* _state;
	Stack* _stack;
	Stack* _stackClosure;

	InstructionValue* _curInsVal;

public:
	void execute();
	void execute_frame();
	typedef std::vector<Instruction*> VtIns;
	int runCode(InstructionValue*);

private:
	void generateClosure(Instruction* ins);

	void add_global_table();
	void enterClosure();
	void quitClosure();
	
	void get_table(Instruction* ins);

	void call(Instruction* ins);
	void passFunParam(Instruction* ins);
	void initLocalVar(Instruction* ins);
	void assignOperate(Instruction* ins);
	void assignVals(int, int, int type);
	void assignSimple(int type);
	void pushValue(Instruction* ins);
	void setLoacalVar(Instruction* ins);
	void getLoacalVar(Instruction* ins);
	void funcionRet(Instruction* ins);
	void operateNum(Instruction* ins);
	void operateLogic(Instruction* ins);
	void registerFunc();
	Closure* getCurrentClosure();

	void ifCompare(Instruction* ins);
	void forCompare(Instruction* ins);
	void breakFor(Instruction* ins);
	void enterBlock(Instruction* ins);
	void quitBlock(Instruction* ins);

	void generateBlock(Instruction* ins);

	void runBlockCode(Value* val);

	void tableDefine(Instruction* ins);
	void tableAccess(Instruction* ins);
	void tableArrIndex(Instruction* ins);

	void negNumber(Instruction* ins);

};

