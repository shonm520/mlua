#pragma  once
#include "GramTreeNode.h"

class Visitor  {
public:
	~Visitor(){}
	virtual void visit(ChunkNode *, void *) = 0;
	virtual void visit(BlockNode *, void *) = 0;
	virtual void visit(SyntaxTreeNodeBase*, void*) = 0;
	virtual void visit(AssignStatement* stm, void* data) = 0;
	virtual void visit(LocalNameListStatement* nls, void* data) = 0;
	virtual void visit(UnaryExpression* uexp, void* data) = 0;
	virtual void visit(Terminator* ter, void* data) = 0;
	virtual void visit(IdentifierNode* idt, void* data) = 0;
	virtual void visit(NormalCallFunciton* callFun, void* data) = 0;
	virtual void visit(OperateStatement* ops, void* data) = 0;
	virtual void visit(FunctionStatement* fsm, void* data) = 0;
	virtual void visit(ReturnStatement* rtSmt, void* data) = 0;
};