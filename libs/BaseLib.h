#pragma  once

struct Value;

class BaseLib  {
public:
	static void PrintType(Value* val);
	static int Print(State* state, void* num_);
	static int generatePairs(State* state, void*);
	static int generateIPairs(State* state, void*);
	static int next(State* state, void*);
	static int type(State* state, void*);
	static int len(State* state, void*);
	static int setmeta(State* state, void*);


	struct StringLib  {
		static int upper(State* state, void*);
		static int substr(State* state, void*);
		static int byte(State* state, void*);
		static int _char(State* state, void*);
		static Table* generateStringTable();
	};



	struct MathLib  {
		static int _pow(State*, void*);
		static int _random(State*, void*);
		static Table* generateMathTable();
	};

	
};
