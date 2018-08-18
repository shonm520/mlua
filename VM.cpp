#include "VM.h"
#include <assert.h>
#include "Value.h"
#include "State.h"
#include "Function.h"
#include "Stack.h"






void PrintType(Value* val)
{
	if (val->Type() == Value::TYPE_STRING)  {
		printf("%s", ((String*)val)->Get().c_str());
	}
	else if (val->Type() == Value::TYPE_NUMBER)  {
		if (((Number*)val)->IsInteger())  {
			printf("%d", ((Number*)val)->GetInteger());
		}
		else  {
			printf("%f", ((Number*)val)->Get());
		}
	}
	else if (val->Type() == Value::TYPE_NIL)  {
		printf("nil");
	}
	else if (val->Type() == Value::TYPE_CLOSURE)  {
		printf("funciont: 0x%08x", val);
	}
}





int Print(State* state, void* num)
{
	std::vector<Value*> vtVals;
	for (int i = (int)num - 1; i >= 0; i--)   {
		Value* val = state->getStack()->popValue();
		vtVals.push_back(val);
	}

	for (int i = (int)num - 1; i >= 0; i--)   {
		Value* val = vtVals[i];
		if (val)  {
			PrintType(val);
		}
		printf("\t");
	}
	printf("\n");
	return 0;
}






VM::VM(State* state)
	: _state(state),
	_stackClosure(new Stack(20))
{
	_stack = _state->getStack();
	registerFunc();
}


VM::~VM()
{
}

void VM::execute()
{
}


void VM::execute_frame()
{

}


void VM::registerFunc()
{
	_state->registerFunc(Print);
}


void VM::runCode(VtIns& vtIns)
{
	for (auto it = vtIns.begin(); it != vtIns.end(); ++it)  {
		Instruction* ins = *it;
		switch (ins->op_code)
		{
		case Instruction::OpCode_AddGlobalTable:
			add_global_table();
			break;
		case Instruction::OpCode_EnterClosure:         //进入函数时调用
			enterClosure();
			break;

		case Instruction::OpCode_QuitClosure:
			quitClosure();
			break;

		case Instruction::OpCode_InitLocalVar:         //局部变量声明
			initLocalVar(ins);
			break;
		case Instruction::OpCode_GetLocalVar:             
			getLoacalVar(ins);
			break;

		case Instruction::OpCode_SetLocalVar:             
			setLoacalVar(ins);
			break;

		case Instruction::OpCode_GenerateClosure:
			generateClosure(ins);
			break;

		case Instruction::OpCode_Call:
			call(ins);
			break;

		case Instruction::OpCode_Ret:
			funcionRet(ins);
			break;

		case Instruction::OpCode_Push:
			pushValue(ins);
			break;

		case Instruction::OpCode_Assign:
			assignOperate(ins);
			break;

		case Instruction::OpCode_Plus:
		case Instruction::OpCode_Minus:
		case Instruction::OpCode_Multiply:
		case Instruction::OpCode_Divide:
			operateNum(ins);
			break;
		
		default:
			break;
		}
	}
}


void VM::generateClosure(Instruction* ins)
{
	assert(ins->param_a.param.value->Type() == Value::TYPE_FUNCTION);
	Function *func = static_cast<Function *>(ins->param_a.param.value);
	Closure *cl = func->generateClosure(_state) ;
	cl->setParentClosure(getCurrentClosure());
	_stack->Push(cl);
}



void VM::add_global_table()
{

}


void VM::call(Instruction* ins)
{

	int paramNum = ins->param_a.param.counter.counter1;
	Value* callee = _stack->popValue();

	if (callee->Type() == Value::TYPE_NIL)  {
		printf("error, call a nil val");
	}

	if (callee->Type() == Value::TYPE_NATIVE_FUNCTION)  {
		((NativeFunc*)callee)->doCall(_state, (void*)paramNum);
	}
	else if (callee->Type() == Value::TYPE_CLOSURE)  {
		Closure* cl = static_cast<Closure*>(callee);
		std::vector<Instruction*> vtIns = cl->getPrototype()->_opcodes;
		_stackClosure->Push(cl);
		runCode(vtIns);
		int retNum = cl->getPrototype()->getRetNum();
		int needNum = ins->param_a.param.counter.counter2;
		int n = retNum - needNum;                         //需要弹出多少个返回值
		while (n > 0)  {     
			_stack->popValue();
			n--;
		}
	}
}

void VM::enterClosure()
{
	getCurrentClosure()->initTables();
};

void VM::quitClosure()
{
	_stackClosure->popValue();
}

Closure* VM::getCurrentClosure()
{
	if (_stackClosure->Size() == 0)  {
		return nullptr;
	}
	Closure* cl = (Closure*)_stackClosure->Top()->param.value;
	assert(cl->Type() == Value::TYPE_CLOSURE);
	return cl;
}


void VM::assignVals(int num_key, int num_val, int type)      //函数调用传入参数时也会调用这里
{
	Table* tab = getCurrentClosure()->getTopTable();
	std::list<Value*> listKeys;
	std::list<Value*> listVals;
	for (int i = 0; i < num_key; i++)  {
		listKeys.push_front(_stack->popValue());
	}
	if (num_key > num_val)  {     //值少于key，有可能是函数引起的a，b =f()
		num_val = _stack->Size();   
	}
	for (int i = 0; i < num_val; i++)    {
		Value* val = _stack->popValue();
		listVals.push_front(val);
	}
	num_val = listVals.size();
	if (num_key > num_val)  {     //a,b,c = 1, 2
		for (int i = 0; i < num_key - num_val; i++)  {
			listVals.push_back(new Nil());
		}
	}

	while (!listKeys.empty() && !listVals.empty())  {     //有可能有多余的参数被舍弃
		Value* key = listKeys.front();
		Value* val = listVals.front();
		if (type == 1)  {   
			if (getCurrentClosure()->findUpTables(key, nullptr, &tab) == -1)  {      //赋值时没有找到就放在全局表中
				tab = _state->getGlobalTable();
			}
		}  
		tab->Assign(key, val);
		listKeys.pop_front();
		listVals.pop_front();
	}
}

void VM::initLocalVar(Instruction* ins)
{
	int num_key = ins->param_a.param.counter.counter1;
	int num_val = ins->param_a.param.counter.counter2;
	if (num_key == 1 && num_val == 1)  {
		assignSimple(0);
	}
	else  {
		assignVals(num_key, num_val, 0);
	}
}

void VM::assignOperate(Instruction* ins)
{
	int num_key = ins->param_a.param.counter.counter1;
	int num_val = ins->param_a.param.counter.counter2;
	if (num_key == 1 && num_val == 1)  {
		assignSimple(1);
	}
	else  {
		assignVals(num_key, num_val, 1);
	}
}

void VM::assignSimple(int type)
{
	Table* tab = getCurrentClosure()->getTopTable();
	Value* key = _stack->popValue();
	Value* val = _stack->popValue();
	if (type == 1 && getCurrentClosure()->findUpTables(key, nullptr, &tab) == -1)  {      //赋值时没有找到就放在全局表中
		tab = _state->getGlobalTable();
	}
	tab->Assign(key, val);
}


void VM::get_table(Instruction* ins)
{
	Value* key = ins->param_a.param.name;
	Table* table = getCurrentClosure()->getTopTable();
	Value* val = table->GetValue(key);
	if (val)  {
		_stack->Push(val);
	}
	
}


void VM::setLoacalVar(Instruction* ins)
{
	_stack->Push(ins->param_a.param.name);
}

void VM::getLoacalVar(Instruction* ins)
{
	Value* key = ins->param_a.param.name;
	Value* val = nullptr;
	if (getCurrentClosure()->findUpTables(key, &val, nullptr) != -1)  {
		_stack->Push(val);
	}
	else  {
		val = new Nil();
		_stack->Push(val);
	}
}

void VM::pushValue(Instruction* ins)
{
	if (ins->param_a.type == InstructionParam::InstructionParamType_Name)  {
		_stack->Push(ins->param_a.param.name);
	}  
	else if (ins->param_a.type == InstructionParam::InstructionParamType_Value)  {
		_stack->Push(ins->param_a.param.value);
	}
}


void VM::operateNum(Instruction* ins)
{
	Value* num1 = _stack->popValue();
	Value* num2 = _stack->popValue();
	
	if (num1->Type() == Value::TYPE_NIL ||
		num2->Type() == Value::TYPE_NIL)  {
		printf("error, operate on a nil value\n");
	}

	double num = 0;
	if (ins->op_code == Instruction_::OpCode_Plus)  {
		num = ((Number*)num2)->Get() + ((Number*)num1)->Get();
	}
	else if (ins->op_code == Instruction_::OpCode_Minus)  {
		num = ((Number*)num2)->Get() - ((Number*)num1)->Get();
	}
	else if (ins->op_code == Instruction_::OpCode_Multiply)  {
		num = ((Number*)num2)->Get() * ((Number*)num1)->Get();
	}
	else if (ins->op_code == Instruction_::OpCode_Divide)  {
		num = ((Number*)num2)->Get() / ((Number*)num1)->Get();
	}
	
	Value* ret = new Number(num);
	_stack->Push(ret);
}


void VM::funcionRet(Instruction* ins)
{
 	int num = ins->param_a.param.counter.counter1;
 	Function* func = getCurrentClosure()->getPrototype();
	func->setRetNum(num);
}