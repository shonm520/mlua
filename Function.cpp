#include "Function.h"
#include "Stack.h"
#include "State.h"




InstructionValue::InstructionValue():
	_insSet(nullptr)
{

}


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
	_parentClosure(nullptr),
	_upTables(nullptr)
{

}

void Closure::initClosure()
{
	clearClosure();
	_nest_tables.push_back(new Table());
}

void Closure::clearClosure()
{
	_nest_tables.clear();
}

void Closure::addBlockTable()
{
	_nest_tables.push_back(new Table());
}

void Closure::removeBlockTable()
{
	Table* top = getTopTable();
	if (top)  {
		delete top;
	}
	_nest_tables.pop_back();
}

Table* Closure::getTopTable()
{
	if (_nest_tables.size() == 0)  {
		return nullptr;
	}
	Table* top = _nest_tables.back();
	return top;
}

void Closure::setParentClosure(Closure* c)
{
	_parentClosure = c;
	if (c)  {
		Table* topTab = c->getTopTable();       //只需拷贝最外层的局部变量,for循环里面的局部变量，子函数是不可见的
		if (topTab)  {
			_upTables = topTab->clone();
		}
	}
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
			return 1;
		}
		cl = cl->_parentClosure;
	}

	if (_upTables)  {      //在其上值中找
		Value* temp = _upTables->GetValue(key);
		if (temp) { 
			if (val) *val = temp;
			if (tab) *tab = _upTables;
			return 2;
		}
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