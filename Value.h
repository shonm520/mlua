#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <math.h>

class  Closure;
class String;
class Function;
struct Value
{
	enum ValueT  {
		TYPE_NIL,
		TYPE_BOOL,
		TYPE_NUMBER,
		TYPE_STRING,
		TYPE_TABLE,
		TYPE_TABLEVAL,
		TYPE_FUNCTION,
		TYPE_CLOSURE,
		TYPE_NATIVE_FUNCTION,
		TYPE_INSTRUCTVAL
	};

	virtual int Type() const = 0;
	virtual std::string Name() const = 0;
	virtual bool IsEqual(const Value *other) const = 0;
	virtual std::size_t GetHash() const = 0;


	struct ValueHasher : public std::unary_function<Value *, std::size_t>  {
		std::size_t operator() (const Value *value) const  {
			return value->GetHash();
		}
	};

	struct ValueEqualer : public std::binary_function<Value *, Value *, bool>  {
		bool operator() (const Value *left, const Value *right) const  {
			return left->IsEqual(right);
		}
	};
};


class Nil : public Value
{
public:
	Nil(){}
	virtual std::string Name() const { return "nil"; }
	virtual int Type() const { return TYPE_NIL; }
	virtual bool IsEqual(const Value *other) const;
	virtual std::size_t GetHash() const { return 0; }
};


class BoolValue : public Value
{
public:
	BoolValue() : _valLogic(false){}
	BoolValue(bool b) : _valLogic(b){}
	virtual std::string Name() const { return "bool"; }
	virtual int Type() const { return TYPE_BOOL; }
	virtual bool IsEqual(const Value *other) const;
	virtual std::size_t GetHash() const { return 0; }

	void setLogicVal(bool b)  { _valLogic = b; }
	bool getLogicVal()  { return _valLogic; }

private:
	bool _valLogic;
};




class String : public Value
{
public:
	explicit String(const std::string& v) : _value(v) {}

	virtual std::string Name() const {	return "string";}
	virtual int Type() const { return TYPE_STRING; }
	virtual bool IsEqual(const Value *other) const;
	std::string Get() { return _value; }
	const std::string& Get() const  { return _value; }

	std::size_t GetHash() const  { return std::hash<std::string>()(_value); }

	String* concat(String* other);
	int getLen() { return _value.length(); }

private:
	std::string _value;
};


class Number : public Value
{
public:
	explicit Number(double v) : _value(v) {}

	virtual std::string Name() const { return "number"; }
	virtual int Type() const { return TYPE_NUMBER; }
	virtual bool IsEqual(const Value *other) const;
	const double Get() const  { return _value; }

	std::size_t GetHash() const  { return std::hash<double>()(_value); }

	int GetInteger() const  { return static_cast<int>(floor(_value)); }
	bool IsInteger() const { return floor(_value) == _value; }
	void SetNumber(double d)  { _value = d; }

private:
	double _value;
};








class TableValue : public Value
{
public:
	friend class Table;
	explicit TableValue(Value *value) : _value(value){}

	virtual int Type() const { return TYPE_TABLEVAL; }
	virtual bool IsEqual(const Value *other) const { return _value->IsEqual(other); };
	virtual std::string Name() const { return _value->Name(); }

	virtual std::size_t GetHash() const  { return _value->GetHash(); }
	Value * GetValue()  { return _value; }
	void SetValue(Value *value)  { _value = value; }

private:
	Value *_value;
};

class Table : public Value
{
public:
	Table();
	virtual std::string Name() const { return "table"; }
	virtual int Type() const { return TYPE_TABLE; }
	bool IsEqual(const Value *other) const  { return this == other; }
	std::size_t GetHash() const  { return std::hash<const Table*>()(this); }

	std::size_t GetArraySize() const;
	bool HaveKey(const Value *key) const;

	Value * GetValue(const Value *key);
	TableValue * GetTableValue(const Value *key);

	void ArrayAssign(std::size_t array_index, Value *value);
	void ArrayAssign(std::size_t array_index, TableValue *table_value);
	void Assign(const Value *key, Value *value);
	void Assign(const Value *key, TableValue *table_value);

	Table* clone();
	int getLen();
	Value* getNextValue(int i, Value** key);
	void setMeta(Table* t)  { _meta = t; }

private:
	typedef std::vector<TableValue *> ArrayType;
	typedef std::unordered_map<const Value *, TableValue *, ValueHasher, ValueEqualer> HashTableType;

	bool HashTableHasKey(const Value *key) const;
	bool ArrayHasKey(const Value *key) const;
	void MarkArray();
	void MarkHashTable();

	ArrayType* _array;
	HashTableType* _hash_table;
	Table* _meta;
	Table* __index;
};

