
#include "../Value.h"
#include "../State.h"
#include "../Function.h"
#include "../Stack.h"
#include "../Parser.h"
#include "../Visitor.h"
#include "../CodeWriter.h"
#include "../CodeGenerate.h"
#include "../VM.h"
#include "BaseLib.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>


void BaseLib::PrintType(Value* val)
{
	if (val->Type() == Value::TYPE_STRING)  {
		printf("%s", ((String*)val)->Get().c_str());
	}
	else if (val->Type() == Value::TYPE_NUMBER)  {
		if (((Number*)val)->IsInteger())  {
			printf("%d", ((Number*)val)->GetInteger());
		}
		else  {
			printf("%.14lg", ((Number*)val)->Get());     //这里可以四舍五入，也可以省略后面的0，完美输出，最多输出后面14位小数
		}
	}
	else if (val->Type() == Value::TYPE_NIL)  {
		printf("nil");
	}
	else if (val->Type() == Value::TYPE_CLOSURE)  {
		printf("funciont: 0x%08x", val);
	}
	else if (val->Type() == Value::TYPE_TABLE)  {
		printf("table: 0x%08x", val);
	}
}


int BaseLib::Print(State* state, void* num_)
{
	std::vector<Value*> vtVals;
	int num = state->getStack()->Size();
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

int BaseLib::generatePairs(State* state, void*)
{
	std::string strPairs =
		"local ipairsIter = function(t, i) \
								i = i + 1\
								local k, v = next(t, i) \
								if v then return k, v end \
							end\
				return ipairsIter";
	Parser parse(strPairs);
	TreeNode* root = parse.parse_block();

	CodeGenerateVisitor codeGen;
	CodeWrite pairs;
	root->accept(&codeGen, &pairs);

	InstructionValue* func = pairs.fetchInstructionVal();
	Value* t = state->getStack()->popValue();
	state->getVM()->runCode(func);
	state->getStack()->Push(t);
	state->getStack()->Push(new Number(-1));
	return 0;
}

int BaseLib::generateIPairs(State* state, void*)
{
	std::string strPairs =
		"local ipairsIter = function(t, i) \
								i = i + 1 \
								local v = t[i] \
								if v then return i, v end \
							end\
				return ipairsIter";
	Parser parse(strPairs);
	TreeNode* root = parse.parse_block();

	CodeGenerateVisitor codeGen;
	CodeWrite pairs;
	root->accept(&codeGen, &pairs);

	InstructionValue* func = pairs.fetchInstructionVal();
	Value* t = state->getStack()->popValue();
	state->getVM()->runCode(func);
	state->getStack()->Push(t);
	state->getStack()->Push(new Number(0));
	return 0;
}


int BaseLib::next(State* state, void*)
{
	Number* num = (Number*)state->getStack()->popValue();
	int i = num->GetInteger();
	Table* tab = (Table*)state->getStack()->popValue();
	Value* key;
	Value* val = tab->getNextValue(i, &key);
	if (i < tab->GetArraySize())  {
		key = new Number(i);
	}
	if (val)  {
		state->getStack()->Push(key);
		state->getStack()->Push(val);
	}
	else  {
		state->getStack()->Push(new Nil());
		state->getStack()->Push(new Nil());
		//printf("invalid key to \'next\'\n");
	}
	return 0;
}

int BaseLib::type(State* state, void*)
{
	Value* val = state->getStack()->popValue();
	if (val->Type() == Value::TYPE_NUMBER)  {
		printf("number\n");
	}
	else if (val->Type() == Value::TYPE_STRING)  {
		printf("string\n");
	}
	else if (val->Type() == Value::TYPE_FUNCTION)  {
		printf("funciton\n");
	}
	else if (val->Type() == Value::TYPE_CLOSURE)  {
		printf("funciton\n");
	}
	else if (val->Type() == Value::TYPE_NATIVE_FUNCTION)  {
		printf("native function\n");
	}
	else if (val->Type() == Value::TYPE_BOOL)  {
		printf("bool\n");
	}
	else if (val->Type() == Value::TYPE_NIL)  {
		printf("nil\n");
	}
	return 0;
}




int BaseLib::StringLib::len(State* state, void*)
{
	state->getVM()->lenOfVale(nullptr);
	return 0;
}

int BaseLib::StringLib::upper(State* state, void*)
{
	String* str = (String*)state->getStack()->popValue();
	int len = str->getLen();
	char* bytes = new char[len + 1];
	bytes[len] = 0;
	for (int i = 0; i < len; i++)  {
		char c = str->Get().at(i);
		if (c >= 'a' && c <= 'z')  {
			c = c - 0x20;
		}
		bytes[i] = c;
	}
	String* up = new String(std::string(bytes));
	delete bytes;
	state->getStack()->Push(up);
	return 0;
}

int BaseLib::StringLib::substr(State* state, void* num)
{
	std::list<Value*> listVals;
	long n = (long)num;
	for (int i = 0; i < n; i++)  {
		listVals.push_back(state->getStack()->popValue());
	}
	int start = 0;
	int cnt = 0;
	Value* val[3] = {0};
	for (int i = 0; i < n; i++)  {
		val[i] = listVals.back();
		listVals.pop_back();
	}
	std::string strRaw;
	if (val[0])  {
		strRaw = ((String*)val[0])->Get();
	}
	if (val[1])  {
		start = ((Number*)val[1])->GetInteger();
	}
	if (val[2])  {
		cnt = ((Number*)val[2])->GetInteger();
	}
	if (cnt < 0 || cnt > strRaw.length())  {
		cnt = strRaw.length();
	}
	if (start >= strRaw.length())  {
		state->getStack()->Push(new Nil());
		return 0;
	}
	std::string sub = strRaw.substr(start, cnt);
	state->getStack()->Push(new String(sub));
	return 0;
}

int BaseLib::StringLib::byte(State* state, void* num)
{
	std::list<Value*> listVals;
	long n = (long)num;
	for (int i = 0; i < n; i++)  {
		listVals.push_back(state->getStack()->popValue());
	}
	int start = 0;
	int end = 0;
	Value* val[3] = { 0 };
	for (int i = 0; i < n; i++)  {
		val[i] = listVals.back();
		listVals.pop_back();
	}
	std::string strRaw;
	if (val[0])  {
		strRaw = ((String*)val[0])->Get();
	}
	if (val[1])  {
		start = ((Number*)val[1])->GetInteger() - 1;
	}
	if (val[2])  {
		end = ((Number*)val[2])->GetInteger() - 1;
	}
	if (end < 0 || end >= strRaw.length())  {
		end = strRaw.length() - 1;
	}
	if (start >= strRaw.length())  {
		state->getStack()->Push(new Nil());
		return 0;
	}
	for (int i = start; i <= end; i++)  {
		char c = strRaw.at(i);
		state->getStack()->Push(new Number(c));
	}
	
	return 0;
}

int BaseLib::StringLib::_char(State* state, void* num)
{
	std::list<Value*> listVals;
	long n = (long)num;
	for (int i = 0; i < n; i++)  {
		listVals.push_back(state->getStack()->popValue());
	}
	char* bytes = new char[n + 1];
	bytes[n] = 0;
	for (int i = 0; i < n; i++)  {
		Number* val = (Number*)listVals.back();
		listVals.pop_back();
		bytes[i] = val->GetInteger();
	}
	std::string strRaw(bytes);
	delete bytes;
	state->getStack()->Push(new String(strRaw));
	return 0;
}

Table* BaseLib::StringLib::generateStringTable()
{
	Table* tab = new Table();
	tab->Assign(new String("len"), new NativeFunc(StringLib::len));
	tab->Assign(new String("upper"), new NativeFunc(StringLib::upper));
	tab->Assign(new String("substr"), new NativeFunc(StringLib::substr));
	tab->Assign(new String("byte"), new NativeFunc(StringLib::byte));
	tab->Assign(new String("char"), new NativeFunc(StringLib::_char));
	return tab;
}







int BaseLib::MathLib::_pow(State* state, void* num)
{
	long n = (long)num;
	if (n != 2)  {
		printf("error, params num error!\n");
	}
	Number* index = (Number*)state->getStack()->popValue();
	Number* base = (Number*)state->getStack()->popValue();
	double ret = pow(base->Get(), index->Get());
	state->getStack()->Push(new Number(ret));
	return 0;
}

int BaseLib::MathLib::_random(State* state, void* num)
{
	long n = (long)num;
	while (n > 0)  {
		state->getStack()->popValue();
		n--;
	}
	srand((unsigned int)time(0));
	int rd = rand() % 10 ;
	state->getStack()->Push(new Number(rd));
	return 0;
}

Table* BaseLib::MathLib::generateMathTable()
{
	Table* tab = new Table();
	tab->Assign(new String("pow"), new NativeFunc(MathLib::_pow));
	tab->Assign(new String("rand"), new NativeFunc(MathLib::_random));
	return tab;
}