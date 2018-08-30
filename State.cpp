#include "State.h"
#include "Value.h"
#include "Function.h"
#include "Stack.h"
#include "libs/BaseLib.h"



State::State():
	_vm(nullptr)
{
	_stack = new Stack(1000);
	_global_table = new Table();
}


State::~State()
{
}




void State::registerFunc(std::string name, Fun fun)
{
	_global_table->Assign(new String(name.c_str()), new NativeFunc(fun));
}


void State::registerTable(std::string name, Table* table)
{
	_global_table->Assign(new String(name.c_str()), table);
}

void State::openLibs()
{
	registerFunc("print", BaseLib::Print);
	registerFunc("pairs", BaseLib::generatePairs);
	registerFunc("ipairs", BaseLib::generateIPairs);
	registerFunc("next", BaseLib::next);
	registerFunc("type", BaseLib::type);

	registerTable("string", BaseLib::StringLib::generateStringTable());
	registerTable("math", BaseLib::MathLib::generateMathTable());
}