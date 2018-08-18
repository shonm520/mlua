#pragma once
#include <list>
#include "Value.h"




class Stack;
struct FuncState  {
	Table* _table;
	Stack* _funcRet;
	FuncState();

};


class State
{
public:
	State();
	~State();

	friend class VM;
	friend class CodeGenerateVisitor;

	Stack* getStack()  { return _stack; }
	Table* getGlobalTable()  { return _global_table; }

	typedef int(*Fun)(State*, void*);
	void registerFunc(Fun);

private:
	Stack* _stack;
	Table* _global_table;
};

