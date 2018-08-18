#include "Function.h"
#include "Stack.h"
#include "State.h"


Function::Function():
	_retNum(0)
{
}


Function::~Function()
{
}




Closure* Function::generateClosure(State* s)
{
	Closure* cl = new Closure(s);
	cl->setPrototype(this);
	return cl;
}





Closure::Closure(State* s)
	:_state(s),
	_prototype(nullptr),
	_parentClosure(nullptr)
{

}

void Closure::initTables()
{
	_nest_tables.clear();
	_nest_tables.push_back(new Table());
}

Table* Closure::getTopTable()
{
	Table* top = _nest_tables.back();
	return top;
}

int Closure::findInNestTables(Value* key, Value** val)
{
	int num = _nest_tables.size();
	int level = 0;
	Value* temp = nullptr;
	for (int i = num - 1; i >= 0; i--)  {
		temp = _nest_tables[i]->GetValue(key);
		if (temp)  {
			if (val) *val = temp;
			return level + 1;
		}
		level++;
	}

	return -1;
}

int Closure::findUpTables(Value* key, Value** val, Table** tab)
{
	Closure* cl = this;
	while (cl)  {
		int ret = cl->findInNestTables(key, val);
		if (ret != -1)  {  //在当前闭包中找到
			if (tab) *tab = cl->getTopTable();
			return 0;
		}
		cl = cl->_parentClosure;
	}

	Table* table = getState()->getGlobalTable();  //已到了最顶层
	Value* temp = table->GetValue(key);
	if (temp)  {
		if (val) *val = temp;
		if (tab) *tab = table;
		return 0;
	}
	else  {
		return -1;
	}
	return -1;
}