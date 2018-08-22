#pragma once
#include "GramTreeNode.h"
#include "Visitor.h"


class CodeWrite;

class CodeGenerateVisitor : public Visitor
{
public:
	CodeGenerateVisitor();
	~CodeGenerateVisitor();

private:

	struct ExpVarData  {
		enum  Oprate_Type{
			VAR_GET,
			VAR_SET
		} ;
		Oprate_Type type;
	};


	struct TableArrayIndex  {
		int num;
	};


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
	void visit(IfStatement* ifSmt, void* data) override;
	void visit(CompareStatement* cmpSmt, void* data);
	void visit(TableDefine* tbdSmt, void* data);
	void visit(TableNameField* tnfSmt, void* data);
	void visit(TabMemberAccessor* tmsSmt, void* data);
	void visit(TabIndexAccessor* tiSmt, void* data);
	void visit(TableArrayFiled* taSmt, void* data);
	void visit(TableIndexField* tifSmt, void* data);
	void visit(ForStatement* forSmt, void* data);

	void generateChunkCode(ChunkNode*, CodeWrite*);
	void generateFuncCode(bool bGlobal, SyntaxTreeNodeBase* name, SyntaxTreeNodeBase* params, SyntaxTreeNodeBase* body, CodeWrite*);
	void generateFuncBodyCode(SyntaxTreeNodeBase*, CodeWrite*);
	void generateClosureCode(InstructionSet*, CodeWrite*);
	void generateNodeListCode(SyntaxTreeNodeBase* exp_list, CodeWrite*, ExpVarData::Oprate_Type type);
};


void CodeGenerate(SyntaxTreeNodeBase* root, State* state);

