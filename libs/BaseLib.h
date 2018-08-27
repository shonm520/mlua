#pragma  once

struct Value;

class BaseLib  {
public:
	static void PrintType(Value* val);
	static int Print(State* state, void* num_);
	static int generatePairs(State* state, void*);
};
