#pragma once
#include <vector>
#include "Instruction.h"

class State;
class Stack;

class VM
{
public:
	VM(State* state);
	~VM();

private:
	State* _state;
	Stack* _stack;
	Stack* _stackClosure;

public:
	void execute();
	void execute_frame();
	//typedef  Instruction_ Instruction;
	typedef std::vector<Instruction*> VtIns;
	void runCode(VtIns&);

private:
	void generateClosure(Instruction* ins);

	void add_global_table();
	void enterClosure();
	void quitClosure();
	
	void get_table(Instruction* ins);

	void call(Instruction* ins);
	void initLocalVar(Instruction* ins);
	void assignOperate(Instruction* ins);
	void assignVals(int, int, int type);
	void assignSimple(int type);
	void pushValue(Instruction* ins);
	void setLoacalVar(Instruction* ins);
	void getLoacalVar(Instruction* ins);
	void funcionRet(Instruction* ins);
	void operateNum(Instruction* ins);
	void registerFunc();
	Closure* getCurrentClosure();

};

