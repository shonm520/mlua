#pragma once
#include <vector>
#include "Instruction.h"
#include "Value.h"


class State;
class Closure;
class Stack;
class Function : public Value
{
public:
	Function();
	~Function();

private:
	int _retNum;     //返回值的数量

public:
	std::vector<Instruction*> _opcodes;

	virtual std::string Name() const { return "function"; }
	virtual int Type() const { return TYPE_FUNCTION; }
	virtual std::size_t GetHash() const  {  return std::hash<const Function *>()(this);}
	virtual bool IsEqual(const Value *other) const  {  return this == other; }

	Closure* generateClosure(State* s);
	void setRetNum(int n) { _retNum = n; }
	int getRetNum() { return _retNum; }

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
	int findInNestTables(Value* key, Value** val);
	int findUpTables(Value* key, Value** val, Table** table);
	void initTables();
	void clearTables();
private:
	
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