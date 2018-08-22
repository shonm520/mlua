#pragma once
#include <vector>
#include "Instruction.h"
#include "Value.h"


class State;
class Closure;
class Stack;


class InstructionValue : public Value
{
public:
	InstructionValue();

	virtual std::string Name() const { return "instruction_val"; }
	virtual int Type() const { return TYPE_INSTRUCTVAL; }
	virtual std::size_t GetHash() const  { return std::hash<const InstructionValue *>()(this); }
	virtual bool IsEqual(const Value *other) const  { return this == other; }


public:
	InstructionSet* getInstructionSet() { return _insSet; }
	void setInstructionSet(InstructionSet* s){ _insSet = s; }
private:
	InstructionSet* _insSet;

};




class Function : public InstructionValue
{
public:
	Function();
	~Function();

public:
	virtual std::string Name() const { return "function"; }
	virtual int Type() const { return TYPE_FUNCTION; }
	virtual std::size_t GetHash() const  {  return std::hash<const Function *>()(this);}
	virtual bool IsEqual(const Value *other) const  {  return this == other; }

	Closure* generateClosure(State* s);

};

class Closure  : public Value
{
public:
	Closure(State*);

	Function * getPrototype() const { return _prototype; }
	void setPrototype(Function *prototype) { _prototype = prototype; }
	State* getState() { return _state; }

	virtual std::string Name() const { return "closure"; }
	virtual int Type() const { return TYPE_CLOSURE; }
	virtual std::size_t GetHash() const  { return _prototype->GetHash(); }
	virtual bool IsEqual(const Value *other) const  { return this == other; }

	void setParentClosure(Closure* c);
	Closure* getParentClosure()  { return _parentClosure; }
	Table* getTopTable();
	int findUpTables(Value* key, Value** val, Table** table);
	void initClosure();
	void clearClosure();
	void balanceStack();

	void addBlockTable();
	void removeBlockTable();

	void setRealRetNum(int n) { _realRetNum = n; }
	void setNeedRetNum(int n)  { _needRetNum = n; }
	int getRealRetNum()  { return _realRetNum; }
	int getNeedRetNum()  { return _needRetNum; }

private:
	int findInNestTables(Value* key, Value** val);
private:
	int _needRetNum;      //函数执行后需要的返回值，例如f()为0个，f() + 1 就为1个
	int _realRetNum;      //函数执行时确实得的返回值，这个也是动态的，比如在if语句中
	State* _state;
	Function* _prototype;
	Closure* _parentClosure;

	typedef std::vector<Table *> NestTables;
	NestTables _nest_tables;
	Table* _upTables;
};


class NativeFunc : public Value
{
public:
	typedef int(*Fun)(State*, void*);

	NativeFunc(Fun f) { _func = f; }
	virtual std::string Name() const { return "native_function"; }
	virtual int Type() const { return TYPE_NATIVE_FUNCTION; }
	virtual std::size_t GetHash() const  { return std::hash<const NativeFunc *>()(this); }
	virtual bool IsEqual(const Value *other) const  { return this == other; }

	int doCall(State* s, void* p) { return _func(s, p); }

private:
	Fun _func;

};

