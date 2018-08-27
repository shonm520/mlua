#pragma once
#include <list>
#include "Value.h"
#include "VM.h"




class Stack;

class State
{
public:
	friend class VM;
	friend class CodeGenerateVisitor;

	State();
	~State();

	Stack* getStack()  { return _stack; }
	Table* getGlobalTable()  { return _global_table; }

	typedef int(*Fun)(State*, void*);
	void registerFunc(std::string name, Fun);
	void setVM(VM* vm)  { _vm = vm; }
	VM* getVM()  { return _vm; }

private:
	Stack* _stack;
	Table* _global_table;
	VM* _vm;
};

