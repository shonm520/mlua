#include "Value.h"
#include <assert.h>




bool String::IsEqual(const Value *other) const
{
	if (this == other)
		return true;

	if (Type() != other->Type())
		return false;

	return _value == static_cast<const String *>(other)->_value;
}





bool Number::IsEqual(const Value *other) const
{
	if (this == other)
		return true;

	if (Type() != other->Type())
		return false;

	return _value == static_cast<const Number *>(other)->_value;
}


bool Nil::IsEqual(const Value *other) const
{
	if (this == other)
		return true;
	return Type() == other->Type();
}
















std::size_t Table::GetArraySize() const
{
	if (_array)
		return _array->size();
	return 0;
}

bool Table::HaveKey(const Value *key) const
{
	if (ArrayHasKey(key))
		return true;

	if (HashTableHasKey(key))
		return true;

	return false;
}

Value * Table::GetValue(const Value *key)
{
	TableValue *table_value = GetTableValue(key);
	if (!table_value)
		return 0;
	return table_value->GetValue();
}

TableValue * Table::GetTableValue(const Value *key)
{
	if (ArrayHasKey(key))  {
		int index = static_cast<const Number *>(key)->GetInteger();
		return (*_array)[index - 1];
	}

	if (HashTableHasKey(key))  {
		HashTableType::iterator it = _hash_table->find(key);
		return it->second;
	}

	return 0;
}



Table::Table() : 
	_hash_table(nullptr), _array(nullptr)
{
	
}

void Table::ArrayAssign(std::size_t array_index, Value *value)
{
	// Array index start from 1
	assert(array_index >= 1);

	std::size_t array_size = GetArraySize();
	if (array_index <= array_size)
		(*_array)[array_index - 1]->SetValue(value);
	else
		ArrayAssign(array_index, new TableValue(value));
}

void Table::ArrayAssign(std::size_t array_index, TableValue *table_value)
{
	// Array index start from 1
	assert(array_index >= 1);

	if (!_array)
		_array = new ArrayType();

	std::size_t array_size = _array->size();
	if (array_index <= array_size)
		(*_array)[array_index - 1] = table_value;
	else if (array_index == array_size + 1)
		_array->push_back(table_value);
	else
		Assign(new Number(array_index), table_value);
}

void Table::Assign(const Value *key, Value *value)
{
	TableValue *tv = GetTableValue(key);
	if (tv)
		tv->SetValue(value);
	else
		Assign(key,  new TableValue(value));
}

void Table::Assign(const Value *key, TableValue *table_value)
{
	if (key->Type() == TYPE_NUMBER)
	{
		const Number *number = static_cast<const Number *>(key);
		if (number->IsInteger())
		{
			int index = number->GetInteger();
			int array_size = GetArraySize();
			if (index >= 1 && index <= array_size + 1)
				return ArrayAssign(index, table_value);
		}
	}

	if (!_hash_table)
		_hash_table = new HashTableType;

	(*_hash_table)[key] = table_value;
}

bool Table::HashTableHasKey(const Value *key) const
{
	if (!_hash_table)
		return false;

	return _hash_table->find(key) != _hash_table->end();
}

bool Table::ArrayHasKey(const Value *key) const
{
	if (!_array)
		return false;

	if (key->Type() != TYPE_NUMBER)
		return false;

	const Number *number = static_cast<const Number *>(key);
	if (!number->IsInteger())
		return false;

	int index = number->GetInteger();
	int size = _array->size();
	if (index < 1 || index > size)
		return false;
	return true;
}

