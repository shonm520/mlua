#include "State.h"
#include "Value.h"
#include "Function.h"
#include "Stack.h"


FuncState::FuncState():
	_table(new Table()),
	_funcRet(new Stack(20))
{

}



State::State()
{
	_stack = new Stack(1000);
	_global_table = new Table();
}


State::~State()
{
}




void State::registerFunc(Fun fun)
{
	_global_table->Assign(new String("print"), new NativeFunc(fun));
}


