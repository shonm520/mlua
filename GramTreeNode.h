#pragma once
#include <string>
#include <map>
#include <stack>
#include <vector>
#include <cassert>
#include <string.h>
using std::string;
using std::stack;
using std::vector;

#include "Scanner.h"
#include "Value.h"
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
		CHUNK_K,
		FOR_K
	};

public:
	SyntaxTreeNodeBase(int nK = None);
	SyntaxTreeNodeBase(Scanner::Token t)  {
		_token = t;
	}
	~SyntaxTreeNodeBase(){}

protected:
	SyntaxTreeNodeBase* _child[Child_Num_Const];
	SyntaxTreeNodeBase* _pParent;
	SyntaxTreeNodeBase* _pNext;
	NodeKind _nodeKind;
	short    _childIndex;     //子类的索引
	short    _siblings;       //同等节点的个数

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

	virtual SyntaxTreeNodeBase* clone();

	virtual void clear(bool next);
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
		_val = nullptr;
	}
	virtual ~IdentifierNode(){}

	void accept(Visitor* visitor, void* data);

	Value* getVal();
private:
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

	Value* getVal();

private:
	TermType _type;
	Value* _val;
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
	SyntaxTreeNodeBase* getVarDecType() { return _child[VarDecNode::VarDec_Type]; }    //变量的声明
	SyntaxTreeNodeBase* getVarDecName() { return _child[VarDecNode::VarDec_Name]; }    //变量的名字

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
	SyntaxTreeNodeBase* getAssginLeft() { return _child[AssignLetf]; }
	SyntaxTreeNodeBase* getAssginRight() { return _child[AssignRight]; }

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

	void accept(Visitor* visitor, void* data);

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

	void accept(Visitor* visitor, void* data);

};


class TabIndexAccessor : public SyntaxTreeNodeBase  {     //table[index]
public:
	TabIndexAccessor() : SyntaxTreeNodeBase()  {
	}
	virtual ~TabIndexAccessor(){}

	void accept(Visitor* visitor, void* data);

};


//t = {1,2,[3]=3, d=5} 1,2是TableArrayFiled,[3]=7是TableIndexField,d=5是TableNameField

class TableIndexField : public SyntaxTreeNodeBase  {   
public:
	TableIndexField() : SyntaxTreeNodeBase()  {
		_nodeKind = TABLE_INDEX_FIELD_K;
	}
	virtual ~TableIndexField(){}

	void accept(Visitor* visitor, void* data);
};

class TableNameField : public SyntaxTreeNodeBase  {    
public:
	TableNameField() : SyntaxTreeNodeBase()  {
		_nodeKind = TABLE_NAME_FIELD_K;
	}
	virtual ~TableNameField(){}

	void accept(Visitor* visitor, void* data);
};

class TableArrayFiled : public SyntaxTreeNodeBase  {    
public:
	TableArrayFiled() : SyntaxTreeNodeBase()  {
		_nodeKind = TABLE_ARRAY_FIELD_K;
	}
	virtual ~TableArrayFiled(){}

	void accept(Visitor* visitor, void* data);
};


class NormalCallFunciton : public SyntaxTreeNodeBase  {    
public:
	NormalCallFunciton() : SyntaxTreeNodeBase()  {
		_nodeKind = FUNCTION_CALL_K;
		_needRetNum = 1;      //一般需要的返回值是1个
	}
	virtual ~NormalCallFunciton(){}

	SyntaxTreeNodeBase* getCaller()  { return _child[0]; }
	SyntaxTreeNodeBase* getParamList()  { return _child[1]; }

	virtual void accept(Visitor* visitor, void* data);

	int _needRetNum;   //需要返回值的个数
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


class NumericForStatement : public SyntaxTreeNodeBase
{
public:
	NumericForStatement() : SyntaxTreeNodeBase()  {
		_nodeKind = SyntaxTreeNodeBase::FOR_K;
	}
	void accept(Visitor* visitor, void* data);

	enum eFor  {
		EStart,	EEnd, EStep,	EBlock
	};
private:
};

class GenericForStatement : public SyntaxTreeNodeBase
{
public:
	GenericForStatement() : SyntaxTreeNodeBase()  {
		_nodeKind = SyntaxTreeNodeBase::FOR_K;
	}
	void accept(Visitor* visitor, void* data);
private:
};

class BreakStatement : public SyntaxTreeNodeBase
{
public:
	BreakStatement() :SyntaxTreeNodeBase()  {
		
	}
	void accept(Visitor* visitor, void* data);
};
