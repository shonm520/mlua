#pragma once
#include <string>
#include <map>
#include <stack>
#include <vector>
#include <cassert>
using std::string;
using std::stack;
using std::vector;

#include "Scanner.h"
#include "Value.h"
//#include "Runtime.h"
#include "Function.h"



class SubroutineBodyNode;
class SubroutineDecNode;
class CompondStatement;
class ClassTreeNode;


class Visitor;




class SyntaxTreeNodeBase
{
public:
	static const int Child_Num_Const = 5;
	static int s_nCurNodeIndex;
	enum NodeKind {
		None, 
		SUBROUTINE_DEC_K,
		BASIC_TYPE_K,
		CLASS_TYPE_K,
		NULL_K,
		PARAM_K,
		VAR_DEC_K,
		ARRAY_K,
		VAR_K,
		IF_STATEMENT_K,
		WHILE_STATEMENT_K,
		CALL_EXPRESSION_K,
		RETURN_STATEMENT_K,
		CALL_STATEMENT_K,
		BOOL_EXPRESSION_K,
		FUNCTION_CALL_K,
		COMPARE_K,
		OPERATION_K,
		BOOL_K,
		NAMELIST_K,    //声明变量
		ASSIGN_K,
		SUBROUTINE_BODY_K,
		BOOL_CONST_K,
		NEGATIVE_K, 
		INT_CONST_K,
		CHAR_CONST_K,
		STRING_CONST_K,
		KEY_WORD_CONST,
		THIS_K,
		TABLE_DEFINE_K,
		TABLE_NAME_FIELD_K,
		TABLE_INDEX_FIELD_K,
		TABLE_ARRAY_FIELD_K,
		FUNCTION_STM_K,
		BLOCK_K,
		CHUNK_K
	};

	static string Kind2Des(int nK)  {
		static map<int, string> mapStrDes;
		if (mapStrDes.size() == 0) {
			mapStrDes.insert(make_pair(SUBROUTINE_DEC_K, "函数定义"));
			mapStrDes.insert(make_pair(BASIC_TYPE_K, "基本类型"));
			mapStrDes.insert(make_pair(PARAM_K, "参数类型"));
			mapStrDes.insert(make_pair(VAR_DEC_K, "变量声明"));
			mapStrDes.insert(make_pair(ARRAY_K, "数组类型"));
			mapStrDes.insert(make_pair(VAR_K, "变量"));
			mapStrDes.insert(make_pair(SUBROUTINE_BODY_K, "函数体"));
			mapStrDes.insert(make_pair(RETURN_STATEMENT_K, "返回语句"));
			mapStrDes.insert(make_pair(CALL_STATEMENT_K, "调用语句"));
			mapStrDes.insert(make_pair(ASSIGN_K, "赋值语句"));
			mapStrDes.insert(make_pair(IF_STATEMENT_K, "if语句"));
			mapStrDes.insert(make_pair(WHILE_STATEMENT_K, "while语句"));
			mapStrDes.insert(make_pair(COMPARE_K, "比较体"));
			mapStrDes.insert(make_pair(OPERATION_K, "操作符"));
			mapStrDes.insert(make_pair(INT_CONST_K, "常整数"));
		}
		return mapStrDes[nK];
	}

public:
	SyntaxTreeNodeBase(int nK = None)  {
		_pNext = nullptr;
		_nodeKind = (NodeKind)nK;
		_strNodeDes = Kind2Des(nK);
		_strClassName = "";
		_childIndex = -1;
		_siblings = 0;
		_pParent = nullptr;
		memset(&_child, 0, sizeof(SyntaxTreeNodeBase*)* Child_Num_Const);

		_nodeIndex = ++s_nCurNodeIndex;
	}
	SyntaxTreeNodeBase(Scanner::Token t)  {
		_token = t;
	}
	~SyntaxTreeNodeBase(){}

protected:
	SyntaxTreeNodeBase* _child[Child_Num_Const];
	SyntaxTreeNodeBase* _pParent;
	SyntaxTreeNodeBase* _pNext;
	NodeKind _nodeKind;
	string   _strNodeDes;
	string   _strClassName;
	short    _childIndex;     //子类的索引
	short    _siblings;       //同等节点的个数
	int      _nodeIndex;      //当前节点的总索引,用于判断变量是否在声明之前使用
	  
	Scanner::Token _token;

public:
	void setToken(Scanner::Token& t)  {
		_token = t;
	}
	Scanner::Token getToken()  {
		return _token;
	}
	const string getLexeme()  {
		return _token.lexeme;
	}
	void setLexeme(string& str)  {
		_token.lexeme = str;
	}
	unsigned int getRow()  {
		return _token.row;
	}
	void setNodeKind(int kind)  {
		_nodeKind = (NodeKind)kind;
	}
	NodeKind getNodeKind()  {
		return _nodeKind;
	}
	short getChildIndex()  {
		return _childIndex;
	}
	int getNodeIndex()  {
		return _nodeIndex;
	}
	void setNextNode(SyntaxTreeNodeBase* node)  {
		if (node)  {
			node->_childIndex = _childIndex;   //这么理解,父亲有多个老婆,每个老婆有0到多个儿子,生母是同一个的孩子编号一样
		}
		_pNext = node;
	}
	short getSiblings()  {
		return _siblings;
	}
	SyntaxTreeNodeBase* getNextNode()  {
		return _pNext;
	}
	SyntaxTreeNodeBase* getParentNode()  {
		return _pParent;
	}
	void setParentNode(SyntaxTreeNodeBase* node)  {   //也要把亲兄弟指定父类
		auto cur = this;
		while (cur)  {
			cur->_pParent = node;
			cur = cur->getNextNode();
		}
	}

	void addChild(SyntaxTreeNodeBase* pChild, int ind);


	SyntaxTreeNodeBase* getChildByIndex(int ind)  {
		if (ind >= 0 && ind < Child_Num_Const)  {
			return _child[ind];
		}
		return nullptr;
	}

	virtual void accept(Visitor* visitor, void* data );

	virtual string getName() { return ""; }
	virtual string getSignName() { return ""; }
	virtual SyntaxTreeNodeBase* getChildByTag(string name) { return nullptr; }

	virtual SyntaxTreeNodeBase* clone() {
		auto node = new SyntaxTreeNodeBase;
		node->_nodeKind = _nodeKind;
		node->_token = _token;
		node->_strClassName = _strClassName;
		node->_pParent = _pParent;
		node->_strNodeDes = _strNodeDes;
		node->_childIndex = _childIndex;
		node->_siblings = _siblings;
		return  node;
	}

	//virtual Value* get_val(){ return nullptr; };


	static SyntaxTreeNodeBase* s_curVarDecType;                                //当前的类型声明,用于解析int a=1,b=2;b的类型

	static stack<ClassTreeNode*> s_stackCurClassZone;                        //当前类的作用域
	static ClassTreeNode* getCurCurClassNode();
	static void insertClassNode(ClassTreeNode* node);
	static void quitClassZone();


	static stack<SubroutineDecNode*> s_stackCurSubroutineZone;               //当前的函数作用域
	static SubroutineDecNode* getCurSubroutineNode();
	static void insertSubRoutineNode(SubroutineDecNode* node);
	static void quitSubRoutineZone();

	static stack<SubroutineBodyNode*> s_stackCurSubroutineBodyZone;          //当前的函数体作用域
	static SubroutineBodyNode* getCurSubroutineBodyNode();
	static void insertSubRoutineBodyNode(SubroutineBodyNode* node);
	static void quitSubRoutineBodyZone();


	static stack<CompondStatement*> s_stackCurCompoundStatmentZone;          //当前符合语句作用域
	static CompondStatement* getCurCompoundStatmentNode(int* pNum = 0);  
	static void insertCompoundStatmentNode(CompondStatement* node);
	static void quitCompoundStatmentZone();

	static bool isInCompound(SyntaxTreeNodeBase* node);       //在整个复合语句,为什么要区分体,因为if(a > 0) {int a = 3;}  这两个作用域不一样
	static bool isInCompoundBody(SyntaxTreeNodeBase* node);   //在复合语句的体
};




typedef SyntaxTreeNodeBase TreeNode;

class TreeNodeList
{
	TreeNode* _head;
	TreeNode* _cur;
public:
	TreeNodeList()  {
		_head = _cur = nullptr;
	}
	void Push(TreeNode* node);
	TreeNode* getHeadNode()  {
		return _head;
	}
	TreeNode* getCurNode()  {
		return _cur;
	}
	TreeNode* joinBy(TreeNodeList* node2);   //合并,第二个接上第一个,返回第一个的头
	static TreeNode* getCurNode(TreeNode* node);
};



class ChunkNode : public SyntaxTreeNodeBase {
public:
	ChunkNode() : SyntaxTreeNodeBase()  {
		_nodeKind = CHUNK_K;
	}
	virtual ~ChunkNode(){}
	void accept(Visitor* visitor, void* data);
};


class BlockNode : public SyntaxTreeNodeBase {
public:
	BlockNode() : SyntaxTreeNodeBase()  {
		_nodeKind = BLOCK_K;
	}
	virtual ~BlockNode(){}

	void accept(Visitor* visitor, void* data);
};

class IdentifierNode : public SyntaxTreeNodeBase  {
public:
	IdentifierNode(Scanner::Token& t) : SyntaxTreeNodeBase()  {
		_token = t;
	}
	virtual ~IdentifierNode(){}

	void accept(Visitor* visitor, void* data);

	Value* get_val()  {
		if (!_val)  {
			_val = new String(_token.lexeme.c_str());
		}
		return _val;
	}
	Value* _val;

};

class Terminator : public SyntaxTreeNodeBase  {
public:
	enum TermType  {
		TERM_NIL,
		TERM_FALSE,
		TERM_TRUE,
		TERM_NUMBER,
		TERM_STRING,
	};

	Terminator(Scanner::Token& t, TermType tt) : SyntaxTreeNodeBase()  {
		_token = t;
		_type = tt;
		_val = nullptr;
	}
	virtual ~Terminator(){}

	void accept(Visitor* visitor, void* data);

	Value* get_val()  {
		if (!_val)  {
			if (_type == TERM_NUMBER)  {
				double num = strtod(_token.lexeme.c_str(), 0);
				_val = new Number(num);
			}
			else  if (_type == TERM_TRUE){
				_val = new BoolValue(true);
			}
			else  if (_type == TERM_FALSE){
				_val = new BoolValue(false);
			}
			else  if (_type == TERM_NIL){
				_val = new Nil();
			}
		}
		return _val;
	}
	Value* _val;

private:
	TermType _type;
};


class UnaryExpression : public SyntaxTreeNodeBase  {
public:
	UnaryExpression(Scanner::Token& t) : SyntaxTreeNodeBase()  {
		_token = t;
	}
	virtual ~UnaryExpression(){}

	void accept(Visitor* visitor, void* data);

};

	
class VarDecNode : public SyntaxTreeNodeBase  {   //变量声明节点
public:
	VarDecNode() : SyntaxTreeNodeBase()  {
		_nodeKind = VAR_DEC_K;
	}
	enum EVarDec  {
		VarDec_Type = 0,
		VarDec_Name
	};
	virtual ~VarDecNode(){}
	SyntaxTreeNodeBase* getVarDecType();    //变量的声明
	SyntaxTreeNodeBase* getVarDecName();    //变量的名字

};


class ParamNode : public VarDecNode  {    //形参节点
public:
	ParamNode() : VarDecNode()  {
		_nodeKind = PARAM_K;
	}
	virtual ~ParamNode(){}
};

class LocalNameListStatement : public SyntaxTreeNodeBase  {      //声明语句
public:
	LocalNameListStatement() : SyntaxTreeNodeBase()  {
		_nodeKind = NAMELIST_K;
	}
	virtual ~LocalNameListStatement() {}

	enum LocalStatement  {
		NameList = 0,
		ExpList
	};

	SyntaxTreeNodeBase* getNameList() { return getChildByIndex(NameList); };
	SyntaxTreeNodeBase* getExpList() { return getChildByIndex(ExpList); };

	void accept(Visitor* visitor, void* data);
};

class AssignStatement : public SyntaxTreeNodeBase  {    
public:
	AssignStatement() : SyntaxTreeNodeBase()  {
		_nodeKind = ASSIGN_K;
	}
	virtual ~AssignStatement(){}
	enum AStatement  {
		AssignLetf = 0,
		AssignRight
	};

	virtual SyntaxTreeNodeBase* getChildByTag(string name) override;
	SyntaxTreeNodeBase* getAssginLeft();
	SyntaxTreeNodeBase* getAssginRight();

	void accept(Visitor* visitor, void* data);
};

class OperateStatement : public SyntaxTreeNodeBase  {
public:
	enum OperateType  {
		Plus,
		Minus,
		Multi,
		Div
	};
	OperateStatement(OperateType type) : SyntaxTreeNodeBase()  {
		_nodeKind = OPERATION_K;
		_opType = type;
	}
	virtual ~OperateStatement(){}

	SyntaxTreeNodeBase* getTermLeft() { return _child[0]; }
	SyntaxTreeNodeBase* getTermRight() { return _child[1]; }

	void accept(Visitor* visitor, void* data);

	OperateType _opType;
};

class TableDefine : public SyntaxTreeNodeBase  {         //talbe节点
public:
	TableDefine() : SyntaxTreeNodeBase()  {
		_nodeKind = TABLE_DEFINE_K;
	}
	virtual ~TableDefine(){}

private:
	TreeNodeList _fieldList;

public:
	void push_filed(TreeNode* node)  {
		_fieldList.Push(node);
	}
	TreeNode* getField()  {
		return _fieldList.getHeadNode();
	}
};


class TabMemberAccessor : public SyntaxTreeNodeBase  {     //table.member
public:
	TabMemberAccessor() : SyntaxTreeNodeBase()  {

	}
	virtual ~TabMemberAccessor(){}

};


class TabIndexAccessor : public SyntaxTreeNodeBase  {     //table[index]
public:
	TabIndexAccessor() : SyntaxTreeNodeBase()  {
	}
	virtual ~TabIndexAccessor(){}

};

class TableIndexField : public SyntaxTreeNodeBase  {    //函数调用节点
public:
	TableIndexField() : SyntaxTreeNodeBase()  {
		_nodeKind = TABLE_INDEX_FIELD_K;
	}
	virtual ~TableIndexField(){}
};

class TableNameField : public SyntaxTreeNodeBase  {    //函数调用节点
public:
	TableNameField() : SyntaxTreeNodeBase()  {
		_nodeKind = TABLE_NAME_FIELD_K;
	}
	virtual ~TableNameField(){}
};

class TableArrayFiled : public SyntaxTreeNodeBase  {    //函数调用节点
public:
	TableArrayFiled() : SyntaxTreeNodeBase()  {
		_nodeKind = TABLE_ARRAY_FIELD_K;
	}
	virtual ~TableArrayFiled(){}
};


class NormalCallFunciton : public SyntaxTreeNodeBase  {    //函数调用节点
public:
	NormalCallFunciton() : SyntaxTreeNodeBase()  {
		_nodeKind = FUNCTION_CALL_K;
		_param = nullptr;
	}
	virtual ~NormalCallFunciton(){}

	SyntaxTreeNodeBase* getCaller()  { return _child[0]; }
	SyntaxTreeNodeBase* getParamList()  { return _child[1]; }

	virtual void accept(Visitor* visitor, void* data);

	void* _param;
};


class FunctionStatement : public SyntaxTreeNodeBase  {
public:
	FunctionStatement() : SyntaxTreeNodeBase()  {
		_nodeKind = FUNCTION_STM_K;
	}
	virtual ~FunctionStatement(){}

	enum FuncStmt  {
		EFuncName,
		EFuncParams,
		EFuncBody,
		EFunRet
	};

	virtual void accept(Visitor* visitor, void* data);
	SyntaxTreeNodeBase* getFuncName() { return _child[EFuncName]; }
	SyntaxTreeNodeBase* getFuncParams() { return _child[EFuncParams]; }
	SyntaxTreeNodeBase* getFuncBody() { return _child[EFuncBody]; }
	SyntaxTreeNodeBase* getFuncRet() { return _child[EFunRet]; }

	void setGlobal(bool b) { _bGlobal = b; }
	bool getGlobal(){ return _bGlobal; }

private:
	bool _bGlobal;
};


class ReturnStatement : public SyntaxTreeNodeBase  {
public:
	ReturnStatement() : SyntaxTreeNodeBase() {
		_nodeKind = RETURN_STATEMENT_K;
	}
	~ReturnStatement(){}

	virtual void accept(Visitor* visitor, void* data);
};



class IfStatement : public SyntaxTreeNodeBase  {
public:
	IfStatement() : SyntaxTreeNodeBase()  {
		_nodeKind = IF_STATEMENT_K;
	}
	enum eIfStmt  {
		ECompare,
		EIf,
		EElseOrEnd
	};

	virtual void accept(Visitor* visitor, void* data);
};

class CompareStatement : public SyntaxTreeNodeBase  {
public:
	CompareStatement() : SyntaxTreeNodeBase()  {
		_nodeKind = COMPARE_K;
	}
	enum eCmp  {
		ECmpLef,
		ECmpRight
	};

	virtual void accept(Visitor* visitor, void* data);
};




class CompondStatement : public SyntaxTreeNodeBase  {    
public:
	CompondStatement(int nK) : SyntaxTreeNodeBase(nK)  {
		insertCompoundStatmentNode(this);
	}

	enum CompondStmt  {
		eExpress = 0,
		eBlockBody
	};

	virtual ~CompondStatement(){}

};


class BaseBlockBody : public SyntaxTreeNodeBase  {          //块,也有变量,语句
public:
	BaseBlockBody() : SyntaxTreeNodeBase()  {
	}
	virtual ~BaseBlockBody(){}

	enum BlockBody  {
		VarDec = 0,
		Statement
	};

	TreeNodeList _statementList;
	TreeNodeList _varDecList;
	void addStatement(SyntaxTreeNodeBase* node)  {
// 		if (getCurCompoundStatmentNode() == nullptr)  {     //没有复合语句(if,while等)才添加子语句,因为在if语句中会作为子语句被添加的
// 			_statementList.Push(node);
// 		}
		_statementList.Push(node);
	}
	void addVarDec(SyntaxTreeNodeBase* node)  {
// 		if (getCurCompoundStatmentNode() == nullptr)  {     //没有复合语句(if,while等)才添加子语句,因为在if语句中会作为子语句被添加的
// 			_varDecList.Push(node);
// 		}
		_varDecList.Push(node);
	}
	void addBodyChild()  {
		SyntaxTreeNodeBase::addChild(_varDecList.getHeadNode(), VarDec);
		SyntaxTreeNodeBase::addChild(_statementList.getHeadNode(), Statement);
	}
};

class CompondStmtBody : public BaseBlockBody  {
public:
	CompondStmtBody() : BaseBlockBody()  {

	}
	virtual ~CompondStmtBody(){}


};



class SubroutineBodyNode;

class SubroutineDecNode : public SyntaxTreeNodeBase  {     //整个函数的节点
public:
	SubroutineDecNode() : SyntaxTreeNodeBase()  {
		_nodeKind = SUBROUTINE_DEC_K;
		insertSubRoutineNode(this);
	}
	virtual ~SubroutineDecNode(){}

	enum  SubroutineFiled  {
		Sign = 0,
		Ret,
		Name,
		Params,
		Body
	};

	virtual string getName() override;
	virtual string getSignName() override;
	string getRetType();
	SyntaxTreeNodeBase* getFirstParam();
	int getFuncLocalsNum();
	SubroutineBodyNode* getSubroutineBody();

	bool hasVarDecInParams(SyntaxTreeNodeBase* node);       //函数的变量是否在参数列表中
};


class SubroutineBodyNode : public BaseBlockBody  {    //函数体节点
public:
	SubroutineBodyNode() : BaseBlockBody()  {
		_nodeKind = SUBROUTINE_BODY_K;
		insertSubRoutineBodyNode(this);
	}
	virtual ~SubroutineBodyNode(){}

	
	bool hasVarDec(SyntaxTreeNodeBase* node);
	VarDecNode* getCurVarDec();

	int getFuncLocalsNum();
};








