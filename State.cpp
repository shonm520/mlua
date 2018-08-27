#include "State.h"
#include "Value.h"
#include "Function.h"
#include "Stack.h"



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


