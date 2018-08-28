
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
			printf("%f", ((Number*)val)->Get());
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