#include "Visitor.h"
#include "GramTreeNode.h"


void NormalCallFunciton::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void SyntaxTreeNodeBase::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void ChunkNode::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void BlockNode::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void LocalNameListStatement::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void AssignStatement::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void UnaryExpression::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void Terminator::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void IdentifierNode::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void OperateStatement::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void FunctionStatement::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void ReturnStatement::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void IfStatement::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void CompareStatement::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void TableDefine::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void TableNameField::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void TableArrayFiled::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void TableIndexField::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void TabMemberAccessor::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void TabIndexAccessor::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void NumericForStatement::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void GenericForStatement::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}

void BreakStatement::accept(Visitor* visitor, void* data)
{
	visitor->visit(this, data);
}



