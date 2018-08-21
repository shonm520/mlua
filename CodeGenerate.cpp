#include "CodeGenerate.h"
#include "Value.h"
#include "State.h"
#include "Function.h"
#include "CodeWriter.h"
#include "VM.h"
#include "Visitor.h"


CodeGenerateVisitor::CodeGenerateVisitor(State* state)
	:_state(state)
{
}


CodeGenerateVisitor::~CodeGenerateVisitor()
{
}


void CodeGenerate(SyntaxTreeNodeBase* root, State* state)
{
	CodeGenerateVisitor codeGen(state);

	CodeWrite boot;
	root->accept(&codeGen, &boot);

	VM vm(state);
	vm.runCode(boot.fetchInstructionSet());
}


void CodeGenerateVisitor::visit(Terminator* term, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	Instruction *ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_Push;
	ins->param_a.type = InstructionParam::InstructionParamType_Value;
	ins->param_a.param.value = term->get_val();
}

void CodeGenerateVisitor::visit(IdentifierNode* idt, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	Instruction* ins = writer->newInstruction();
	ExpVarData::Oprate_Type type = static_cast<ExpVarData*>(writer->paramRW)->type;
	if (type == ExpVarData::VAR_SET)  {                                   //局部变量赋值
		ins->op_code = Instruction::OpCode_SetLocalVar;
		ins->param_a.type = InstructionParam::InstructionParamType_Name;
		ins->param_a.param.name = (String*)idt->get_val();
	}
	else if (type == ExpVarData::VAR_GET)  {
		ins->op_code = Instruction::OpCode_GetLocalVar;
		ins->param_a.type = InstructionParam::InstructionParamType_Name;
		ins->param_a.param.name = (String*)idt->get_val();
	}
}

void CodeGenerateVisitor::visit(SyntaxTreeNodeBase* node, void* data)
{
	if (node->getToken().kind == Scanner::INT)  {

	}
}

void CodeGenerateVisitor::visit(ChunkNode* chunk, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	generateChunkCode(chunk, writer);	
}

void CodeGenerateVisitor::generateChunkCode(ChunkNode* chunk, CodeWrite* writer)
{
	Instruction *ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_AddGlobalTable;

	generateFuncCode(false, nullptr, nullptr, 
		chunk->getChildByIndex(0), writer);

	ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_Call;
	ins->param_a.param.counter.counter1 = 0;

	ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_DelGlobalTable;
}

void CodeGenerateVisitor::generateClosureCode(InstructionSet* func, CodeWrite* writer)
{
	Instruction *ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_GenerateClosure;
	ins->param_a.type = InstructionParam::InstructionParamType_Value;

	Function* valInsSet = new Function();
	valInsSet->setInstructionSet(func);

	ins->param_a.param.value = valInsSet;
}


void CodeGenerateVisitor::generateFuncCode(bool bGlobal, SyntaxTreeNodeBase* name, SyntaxTreeNodeBase* params,
							SyntaxTreeNodeBase* body, CodeWrite* data)
{
	CodeWrite f_writer;
	Instruction *ins = f_writer.newInstruction();
	ins->op_code = Instruction::OpCode_EnterClosure;

	if (params)  {
		generateNodeListCode(params, &f_writer, ExpVarData::VAR_SET);
		ins = f_writer.newInstruction();
		ins->op_code = Instruction::OpCode_InitLocalVar;
		ins->param_a.param.counter.counter1 = params->getSiblings();
	}

	generateFuncBodyCode(body, &f_writer);

	ins = f_writer.newInstruction();
	ins->op_code = Instruction::OpCode_QuitClosure;

	CodeWrite* writer = static_cast<CodeWrite*>(data);
	generateClosureCode(f_writer.fetchInstructionSet(), writer);

	if (name)  {
		ExpVarData ed;
		ed.type = ExpVarData::VAR_SET;
		writer->paramRW = &ed;
		name->accept(this, writer);
		Instruction* ins = writer->newInstruction();
		ins->param_a.param.counter.counter1 = 1;
		ins->param_a.param.counter.counter2 = 1;
		if (bGlobal)  {
			ins->op_code = Instruction::OpCode_Assign;
		}
		else  {
			ins->op_code = Instruction::OpCode_InitLocalVar;
		}
	}
}

void CodeGenerateVisitor::generateFuncBodyCode(SyntaxTreeNodeBase* block, CodeWrite* writer)
{
	block->accept(this, writer);
}

void CodeGenerateVisitor::visit(BlockNode* block, void* data)
{
	auto stm = block->getChildByIndex(0);
	for (; stm != nullptr; stm = stm->getNextNode())  {
		stm->accept(this, data);
	}
}

void CodeGenerateVisitor::visit(UnaryExpression* uexp, void* data)
{
	TreeNode* exp = uexp->getChildByIndex(0);
	exp->accept(this, data);

	CodeWrite* writer = static_cast<CodeWrite*>(data);
	Instruction *ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_ResetCounter;

	ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_Not;
}


void CodeGenerateVisitor::visit(LocalNameListStatement* stm, void* data)
{
	auto name_list = stm->getNameList();
	auto exp_list = stm->getExpList();

	CodeWrite* writer = static_cast<CodeWrite*>(data);
	short n1 = name_list->getSiblings();
	short n2 = exp_list->getSiblings();

	ExpVarData ed = { ExpVarData::VAR_GET };
	int n = 0;
	for (auto exp = exp_list; exp != nullptr; exp = exp->getNextNode())  {
		n++;
		writer->paramRW = &ed;
		if (exp->getNodeKind() == SyntaxTreeNodeBase::FUNCTION_CALL_K)  {
			int fd = 1;
			if (! exp->getNextNode())  {   //如果是最后一个，需要弹出的个数是a,b,c,d = 1,f() =>  4 - 2 + 1
				fd = n1 - n2 + 1;
			}
			if (n > n1) fd = 0;
			((NormalCallFunciton*)exp)->_param = &fd;
		}
		exp->accept(this, writer);
	}

	generateNodeListCode(name_list, writer, ExpVarData::VAR_SET);

	Instruction* ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_InitLocalVar;
	ins->param_a.param.counter.counter1 = n1;
	ins->param_a.param.counter.counter2 = n2;
}

void CodeGenerateVisitor::visit(AssignStatement* stm, void* data)
{
	auto name_list = stm->getAssginLeft();
	auto exp_list = stm->getAssginRight();
	
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	short n1 = name_list->getSiblings();
	short n2 = exp_list->getSiblings();

	ExpVarData ed = { ExpVarData::VAR_GET };
	int n = 0;
	for (auto exp = exp_list; exp != nullptr; exp = exp->getNextNode())  {
		n++;
		writer->paramRW = &ed;
		if (exp->getNodeKind() == SyntaxTreeNodeBase::FUNCTION_CALL_K)  {
			int fd = 1;
			if (! exp->getNextNode())  {   //如果是最后一个，需要弹出的个数是a,b,c,d = 1,f() =>  4 - 2 + 1
				fd = n1 - n2 + 1;
			}
			if (n > n1) fd = 0;
			((NormalCallFunciton*)exp)->_param = &fd;
		}
		exp->accept(this, writer);
	}

	generateNodeListCode(name_list, writer, ExpVarData::VAR_SET);

	Instruction* ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_Assign;
	ins->param_a.param.counter.counter1 = n1;
	ins->param_a.param.counter.counter2 = n2;
}


void CodeGenerateVisitor::visit(NormalCallFunciton* callFun, void* data)
{
	auto caller = callFun->getCaller();
	auto paramList = callFun->getParamList();

	CodeWrite* writer = static_cast<CodeWrite*>(data);
	if (paramList)  {
		generateNodeListCode(paramList, writer, ExpVarData::VAR_GET);
	}

	ExpVarData ed = { ExpVarData::VAR_GET };
	writer->paramRW = &ed;
	caller->accept(this, writer);        //函数名还是放后面吧

	int numParam = ((paramList == nullptr) ? 0 : paramList->getSiblings());

	Instruction *ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_Call;
	ins->param_a.param.counter.counter1 = numParam;

	if (callFun->_param)  {
		int num = static_cast<FuncVarData*>(callFun->_param)->num;
		ins->param_a.param.counter.counter2 = num;
	}
	else  {
		ins->param_a.param.counter.counter2 = 0;
	}
}


void CodeGenerateVisitor::generateNodeListCode(SyntaxTreeNodeBase* node_list, CodeWrite* writer, ExpVarData::Oprate_Type type)
{
	ExpVarData ed = { type };
	for (auto exp = node_list; exp != nullptr; exp = exp->getNextNode())  {
		writer->paramRW = &ed;
		if (exp->getNodeKind() == SyntaxTreeNodeBase::FUNCTION_CALL_K)  {
			FuncVarData fd = { 1 };
			((NormalCallFunciton*)exp)->_param = &fd;
		}
		exp->accept(this, writer);
	}
}


void CodeGenerateVisitor::visit(OperateStatement* ops, void* data)
{
	auto term1 = ops->getTermLeft();
	auto term2 = ops->getTermRight();

	if (term1->getNodeKind() == SyntaxTreeNodeBase::FUNCTION_CALL_K)  {
		FuncVarData_ fd = { 1 };
		((NormalCallFunciton*)term1)->_param = &fd;
	}
	if (term2->getNodeKind() == SyntaxTreeNodeBase::FUNCTION_CALL_K)  {
		FuncVarData_ fd = { 1 };
		((NormalCallFunciton*)term2)->_param = &fd;
	}

	CodeWrite* writer = static_cast<CodeWrite*>(data);
	ExpVarData ed = { ExpVarData::VAR_GET };
	writer->paramRW = &ed;
	term1->accept(this, writer);
	writer->paramRW = &ed;
	term2->accept(this, writer);

	Instruction* ins = writer->newInstruction();
	ins->op_code = (Instruction::OpCode)(Instruction::OpCode_Plus + ops->_opType);
}

void CodeGenerateVisitor::visit(FunctionStatement* fsm, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	generateFuncCode(fsm->getGlobal(), fsm->getFuncName(), fsm->getFuncParams(),
		fsm->getFuncBody(), writer);
}

void CodeGenerateVisitor::visit(ReturnStatement* rtSmt, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	auto rtList = rtSmt->getChildByIndex(0);
	generateNodeListCode(rtList, writer, ExpVarData::VAR_GET);

	Instruction* ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_Ret;
	ins->param_a.param.counter.counter1 = rtList->getSiblings();
}

void CodeGenerateVisitor::visit(IfStatement* ifSmt, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	Instruction* ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_EnterBlock;

	BlockNode* ifSmtRight = (BlockNode*)ifSmt->getChildByIndex(IfStatement::EElseOrEnd);
	if (ifSmtRight)  {
		CodeWrite br_writer;
		ifSmtRight->accept(this, &br_writer);
		ins = writer->newInstruction();
		ins->op_code = Instruction::OpCode_GenerateBlock;
		ins->param_a.type = InstructionParam::InstructionParamType_Value;
		InstructionValue* valInsSet = br_writer.fetchInstructionVal();
		ins->param_a.param.value = valInsSet;
	}

	BlockNode* ifSmtLeft = (BlockNode*)ifSmt->getChildByIndex(IfStatement::EIf);
	CodeWrite bl_writer;
	ifSmtLeft->accept(this, &bl_writer);
	ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_GenerateBlock;
	ins->param_a.type = InstructionParam::InstructionParamType_Value;
	ins->param_a.param.value = bl_writer.fetchInstructionVal();
	ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_QuitBlock;

	auto cmp = ifSmt->getChildByIndex(IfStatement::ECompare);     //逻辑比较部分放在最后
	ExpVarData ed = { ExpVarData::VAR_GET };
	writer->paramRW = &ed;
	cmp->accept(this, data);

	ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_If;
	ins->param_a.param.counter.counter1 = (ifSmtRight == 0) ? 1 : 2;
}

void CodeGenerateVisitor::visit(CompareStatement* cmpSmt, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	ExpVarData ed = { ExpVarData::VAR_GET };
	writer->paramRW = &ed;
	auto left = cmpSmt->getChildByIndex(CompareStatement::ECmpLef);
	left->accept(this, data);

	auto right = cmpSmt->getChildByIndex(CompareStatement::ECmpRight);
	ed = { ExpVarData::VAR_GET };
	writer->paramRW = &ed;
	right->accept(this, data);

	Instruction* ins = writer->newInstruction();
	Scanner::Token t = cmpSmt->getToken();

	std::string strCmp[] = {"<", ">", "<=", ">=", "=", "=="};
	for (int i = 0; i < 6; i++)  {
		if (t.lexeme == strCmp[i])  {
			ins->op_code = (Instruction::OpCode)(Instruction::OpCode_Less + i);
		}
	}
}