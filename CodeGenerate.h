#pragma once
#include "GramTreeNode.h"
#include "Visitor.h"
#include <stack>
#include <map>
class SyntaxTreeNodeBase;
class State;
class Function;
struct GenerateBlock;
struct GenerateFunction;

using std::stack;
using std::map;

class CodeWrite;

class CodeGenerateVisitor : public Visitor
{
public:
	CodeGenerateVisitor(State *state);
	~CodeGenerateVisitor();

private:
	State* _state;

	GenerateFunction* _current_function;

	class Info
	{
	public:
		string type;    // int, float, char, string
		int index;
		int nodeIndex;  //当前节点的总索引,用于判断变量是否在声明之前使用
		vector<string> args;
	};
	static Info None;

	class MapInfo : public map<string, Info>  {
	public:
		unsigned int index;
		string strAreaType;
	};
	stack<MapInfo*> _stackMapBlockTable;


	struct ExpVarData  {
		enum  Oprate_Type{
			VAR_GET,
			VAR_SET
		} ;
		Oprate_Type type;
	};


	typedef struct FuncVarData_  {
		int num;
	}FuncVarData;


public:

	void visit(ChunkNode* block, void* data) override;
	void visit(BlockNode* block, void* data) override;
	void visit(SyntaxTreeNodeBase* block, void* data) override;
	void visit(LocalNameListStatement* stm, void* data) override;
	void visit(AssignStatement* stm, void* data) override;
	void visit(UnaryExpression* uexp, void* data) override;
	void visit(Terminator* uexp, void* data) override;
	void visit(IdentifierNode* idt, void* data) override;
	void visit(NormalCallFunciton* block, void* data) override;
	void visit(OperateStatement* ops, void* data) override;
	void visit(FunctionStatement* fsm, void* data) override;
	void visit(ReturnStatement* rtSmt, void* data) override;

	void enter_function();       //在访问chunk和function body会进入
	void enter_block();

	Function* get_current_function() const;
	MapInfo* get_current_block_mapInfo() const;


	void generateChunkCode(ChunkNode*, CodeWrite*);
	//void generate_closure(CodeWrite*);
	void generate_func_name(CodeWrite*);

	void generateFuncCode(bool bGlobal, SyntaxTreeNodeBase* name, SyntaxTreeNodeBase* params, SyntaxTreeNodeBase* body, Function*,  CodeWrite*);
	void generateFuncBodyCode(SyntaxTreeNodeBase*, CodeWrite*);
	void generateClosureCode(Function*, CodeWrite*);

	//void generate_explist_code(SyntaxTreeNodeBase* exp_list, CodeWrite*);
	void generate_nodelist_code(SyntaxTreeNodeBase* exp_list, CodeWrite*, ExpVarData::Oprate_Type type);
	//void generate_namelist_code(SyntaxTreeNodeBase* name_list, CodeWrite*);

	void generate_assign_code(SyntaxTreeNodeBase* name_list, SyntaxTreeNodeBase* exp_list, CodeWrite*);
};

struct GenerateBlock
{
	GenerateBlock *parent_;
	// Current block register start id
	int register_start_id_;
	// Local names
	// Same names are the same instance String, so using String
	// pointer as key is fine
	//std::unordered_map<String *, LocalNameInfo> names_;

	// Current loop ast info
	//LoopInfo current_loop_;

	GenerateBlock() : parent_(nullptr), register_start_id_(0) { }

	//stack<string, LocalNameInfo> _stack;
};



struct GenerateFunction
{
	GenerateFunction *parent_;
	// Current block
	GenerateBlock *current_block_;
	// Current function for code generate
	Function *function_;
	// Index of current function in parent
	int func_index_;
	// Register id generator
	int register_id_;
	// Max register count used in current function
	int register_max_;
	// To be filled loop jump info
	//std::list<LoopJumpInfo> loop_jumps_;

	GenerateFunction()
		: parent_(nullptr), current_block_(nullptr),
		function_(nullptr), func_index_(0),
		register_id_(0), register_max_(0) { }
};




void CodeGenerate(SyntaxTreeNodeBase* root, State* state);

