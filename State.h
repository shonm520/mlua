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

	VM* getVM()  { return _vm; }
	void setVM(VM* vm)  { _vm = vm; }
	Stack* getStack()  { return _stack; }
	Table* getGlobalTable()  { return _global_table; }

	typedef int(*Fun)(State*, void*);
	void registerFunc(std::string name, Fun);
	void registerTable(std::string name, Table* table);
	void openLibs();

private:
	Stack* _stack;
	Table* _global_table;
	VM* _vm;
};

