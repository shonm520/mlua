#include "CodeGenerate.h"
#include "Value.h"
#include "State.h"
#include "Function.h"
#include "CodeWriter.h"
#include "VM.h"
#include "Visitor.h"


CodeGenerateVisitor::CodeGenerateVisitor()
{
}


CodeGenerateVisitor::~CodeGenerateVisitor()
{
}


void CodeGenerate(SyntaxTreeNodeBase* root, State* state)
{
	CodeGenerateVisitor codeGen;

	CodeWrite boot;
	root->accept(&codeGen, &boot);

	VM vm(state);
	vm.runCode(boot.fetchInstructionVal());
}


void CodeGenerateVisitor::visit(Terminator* term, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	Instruction *ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_Push;
	ins->type = InstructionParam::InstructionParamType_Value;
	ins->param.value = term->get_val();
}

void CodeGenerateVisitor::visit(IdentifierNode* idt, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	Instruction* ins = writer->newInstruction();
	ExpVarData::Oprate_Type type = static_cast<ExpVarData*>(writer->paramRW)->type;
	if (type == ExpVarData::VAR_SET)  {                                   //局部变量赋值
		ins->op_code = Instruction::OpCode_SetLocalVar;
		ins->type = InstructionParam::InstructionParamType_Name;
		ins->param.name = (String*)idt->get_val();
	}
	else if (type == ExpVarData::VAR_GET)  {
		ins->op_code = Instruction::OpCode_GetLocalVar;
		ins->type = InstructionParam::InstructionParamType_Name;
		ins->param.name = (String*)idt->get_val();
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
	ins->param.counter.counter1 = 0;

	ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_DelGlobalTable;
}

void CodeGenerateVisitor::generateClosureCode(InstructionSet* func, CodeWrite* writer)
{
	Instruction *ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_GenerateClosure;
	ins->type = InstructionParam::InstructionParamType_Value;

	Function* valInsSet = new Function();
	valInsSet->setInstructionSet(func);
	ins->param.value = valInsSet;
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
		ins->op_code = Instruction::OpCode_PassFunParam;
		ins->param.counter.counter1 = params->getSiblings();
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
		ins->param.counter.counter1 = 1;
		ins->param.counter.counter2 = 1;
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
		if (stm->getNodeKind() == SyntaxTreeNodeBase::FUNCTION_CALL_K)  {    //函数调用作为单独的表达式时，肯定是不需要返回值的
			((NormalCallFunciton*)stm)->_needRetNum = 0;
		}
		stm->accept(this, data);
	}
}

void CodeGenerateVisitor::visit(UnaryExpression* uexp, void* data)
{
	TreeNode* exp = uexp->getChildByIndex(0);
	exp->accept(this, data);

	CodeWrite* writer = static_cast<CodeWrite*>(data);
	Instruction* ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_Negative;
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
			((NormalCallFunciton*)exp)->_needRetNum = fd;
		}
		exp->accept(this, writer);
	}

	generateNodeListCode(name_list, writer, ExpVarData::VAR_SET);

	Instruction* ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_InitLocalVar;
	ins->param.counter.counter1 = n1;
	ins->param.counter.counter2 = n2;
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
			((NormalCallFunciton*)exp)->_needRetNum = fd;
		}
		exp->accept(this, writer);
	}

	generateNodeListCode(name_list, writer, ExpVarData::VAR_SET);    //先表达式，后名字

	Instruction* ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_Assign;
	ins->param.counter.counter1 = n1;
	ins->param.counter.counter2 = n2;
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
	ins->param.counter.counter1 = numParam;
	ins->param.counter.counter2 = callFun->_needRetNum;
}

void CodeGenerateVisitor::generateNodeListCode(SyntaxTreeNodeBase* node_list, CodeWrite* writer, ExpVarData::Oprate_Type type)
{
	ExpVarData ed = { type };
	auto func = [=](TreeNode* exp, bool last)  {
		writer->paramRW = (void*)&ed;
		if (exp->getNodeKind() == SyntaxTreeNodeBase::FUNCTION_CALL_K)  {
			((NormalCallFunciton*)exp)->_needRetNum = 1;
			if (last)  {
				((NormalCallFunciton*)exp)->_needRetNum = -1;
			}
		}
		exp->accept(this, writer);
	};

	for (auto exp = node_list; exp != nullptr; exp = exp->getNextNode())  {
		func(exp, (exp->getNextNode() == nullptr));
	}
}

void CodeGenerateVisitor::visit(OperateStatement* ops, void* data)
{
	auto term1 = ops->getTermLeft();
	auto term2 = ops->getTermRight();

	if (term1->getNodeKind() == SyntaxTreeNodeBase::FUNCTION_CALL_K)  {
		((NormalCallFunciton*)term1)->_needRetNum = 1;
	}
	if (term2->getNodeKind() == SyntaxTreeNodeBase::FUNCTION_CALL_K)  {
		((NormalCallFunciton*)term2)->_needRetNum = 1;
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
	ins->param.counter.counter1 = rtList->getSiblings();
}

void CodeGenerateVisitor::visit(IfStatement* ifSmt, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	BlockNode* ifSmtRight = (BlockNode*)ifSmt->getChildByIndex(IfStatement::EElseOrEnd);
	if (ifSmtRight)  {
		CodeWrite br_writer;
		ifSmtRight->accept(this, &br_writer);
		Instruction* ins = writer->newInstruction();
		ins->op_code = Instruction::OpCode_GenerateBlock;
		ins->type = InstructionParam::InstructionParamType_Value;
		InstructionValue* valInsSet = br_writer.fetchInstructionVal();
		ins->param.value = valInsSet;
	}

	BlockNode* ifSmtLeft = (BlockNode*)ifSmt->getChildByIndex(IfStatement::EIf);
	CodeWrite bl_writer;
	ifSmtLeft->accept(this, &bl_writer);
	Instruction* ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_GenerateBlock;
	ins->type = InstructionParam::InstructionParamType_Value;
	ins->param.value = bl_writer.fetchInstructionVal();

	auto cmp = ifSmt->getChildByIndex(IfStatement::ECompare);     //逻辑比较部分放在最后
	ExpVarData ed = { ExpVarData::VAR_GET };
	writer->paramRW = &ed;
	cmp->accept(this, data);

	ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_If;
	ins->param.counter.counter1 = (ifSmtRight == 0) ? 0 : 1;
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

void CodeGenerateVisitor::visit(TableDefine* tbdSmt, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	auto filedList = tbdSmt->getField();
	int num = 0;
	std::vector<TreeNode*> vtNodes;
	std::vector<TreeNode*> vtArrayNodes;
	for (auto exp = filedList; exp != nullptr; exp = exp->getNextNode())  {
		if (exp->getNodeKind() == SyntaxTreeNodeBase::TABLE_ARRAY_FIELD_K)  {
			vtArrayNodes.push_back(exp);
		}
		else  {  
			vtNodes.push_back(exp);      //TableNameField, TableIndexField
		}
		num++;
	}
	int arrNum = vtArrayNodes.size();        //让TableArrayFiled先执行，因为他要覆盖[]
	for (auto it = vtArrayNodes.rbegin(); it != vtArrayNodes.rend(); ++it)  {      //arr index还是从后边解析吧，因为从栈上取出的方向相反
		TableArrayIndex ti = { arrNum-- };
		writer->paramArrInd = &ti;
		(*it)->accept(this, data);
	}

	for (auto it = vtNodes.rbegin(); it != vtNodes.rend(); ++it)  {      //arr index还是从后边解析吧，因为从栈上取出的方向相反
		(*it)->accept(this, data);
	}

	Instruction* ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_TableDefine;
	ins->param.counter.counter1 = num;
}

void CodeGenerateVisitor::visit(TableNameField* tnfSmt, void* data)   //t = {1,2,[3]=3, d=5} 1,2是TableArrayFiled,[3]=7是TableIndexField,d=5是TableNameField
{
	auto key = tnfSmt->getChildByIndex(0);
	auto val = tnfSmt->getChildByIndex(1);

	CodeWrite* writer = static_cast<CodeWrite*>(data);
	ExpVarData ed = { ExpVarData::VAR_GET };
	writer->paramRW = &ed;
	val->accept(this, data);

	ed.type = ExpVarData::VAR_SET;
	writer->paramRW = &ed;
	key->accept(this, data);
}

void CodeGenerateVisitor::visit(TableArrayFiled* taSmt, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	int index = ((TableArrayIndex*)writer->paramArrInd)->num;
	ExpVarData ed = { ExpVarData::VAR_GET };
	writer->paramRW = &ed;
	taSmt->getChildByIndex(0)->accept(this, data);
	Instruction* ins = writer->newInstruction();

	ins->op_code = Instruction::OpCode_Push;
	ins->type = InstructionParam::InstructionParamType_Value;
	ins->param.value = new Number(index);
}

void CodeGenerateVisitor::visit(TableIndexField* tifSmt, void* data)
{
	visit((TableNameField*)tifSmt, data);
}

void CodeGenerateVisitor::visit(TabMemberAccessor* tmsSmt, void* data)
{
	auto tab = tmsSmt->getChildByIndex(0);
	auto member = tmsSmt->getChildByIndex(1);
	
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	ExpVarData ed = { ExpVarData::VAR_SET };      //读取表的key，val时，必须是set,将表名压入栈中
	writer->paramRW = &ed;
	member->accept(this, data);

	ed.type = ExpVarData::VAR_SET;
	writer->paramRW = &ed;
	tab->accept(this, data);

	Instruction* ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_TableMemAccess;
	ins->param.value = new String(member->getLexeme());
}

void CodeGenerateVisitor::visit(TabIndexAccessor* tiSmt, void* data)
{
	auto tab = tiSmt->getChildByIndex(0);
	auto member = tiSmt->getChildByIndex(1);

	CodeWrite* writer = static_cast<CodeWrite*>(data);
	ExpVarData ed = { ExpVarData::VAR_GET };      //t[k],k必须是get
	writer->paramRW = &ed;
	member->accept(this, data);

	ed.type = ExpVarData::VAR_SET;
	writer->paramRW = &ed;
	tab->accept(this, data);

	Instruction* ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_TableMemAccess;
	ins->param.value = new String(member->getLexeme());
}

void CodeGenerateVisitor::visit(NumericForStatement* forSmt, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);

	auto start = forSmt->getChildByIndex(NumericForStatement::EStart);
	auto end = forSmt->getChildByIndex(NumericForStatement::EEnd);
	auto step = forSmt->getChildByIndex(NumericForStatement::EStep);
	auto block = forSmt->getChildByIndex(NumericForStatement::EBlock);

	Instruction* ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_EnterBlock;    //必须有这个，for后面的都是局部变量，for循环体中的更加是

	CodeWrite for_writer;
	block->accept(this, &for_writer);

	if (step) step->accept(this, data);
	if (end) end->accept(this, data);
	if (start)   {
		start->accept(this, data);
		ExpVarData ed = { ExpVarData::VAR_SET };
		writer->paramRW = &ed;
		start->getChildByIndex(0)->accept(this, data);   //for循环变量名仍需压栈
	}

	ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_NumericFor;
	ins->param.value = for_writer.fetchInstructionVal();
	ins->param.counter.counter1 = (step == 0) ? 0 : 1;    //递增还是递减

	ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_QuitBlock;
}

void CodeGenerateVisitor::visit(GenericForStatement* gforSmt, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);

	auto nameList = gforSmt->getChildByIndex(0);
	auto expList = gforSmt->getChildByIndex(1);
	auto block = gforSmt->getChildByIndex(2);

	Instruction* ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_EnterBlock;    //必须有这个，for后面的都是局部变量，for循环体中的更加是

	CodeWrite for_writer;
	block->accept(this, &for_writer);
	InstructionValue* valInsSet = for_writer.fetchInstructionVal();

	ExpVarData ed = { ExpVarData::VAR_GET };
	
	writer->paramRW = &ed;
	if (expList->getNodeKind() == NormalCallFunciton::FUNCTION_CALL_K)  {
		((NormalCallFunciton*)expList)->_needRetNum = 3;
	}
	expList->accept(this, data);       //这个是迭代函数

	generateNodeListCode(expList->getNextNode(), writer, ed.type);   //压入参数

	generateNodeListCode(nameList, writer, ExpVarData::VAR_SET);     //k,v

	ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_GenericFor;
	ins->param.value = valInsSet;
	ins->param.counter.counter1 = nameList->getSiblings();

	ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_QuitBlock;
}

void CodeGenerateVisitor::visit(BreakStatement* brkSmt, void* data)
{
	CodeWrite* writer = static_cast<CodeWrite*>(data);
	Instruction* ins = writer->newInstruction();
	ins->op_code = Instruction::OpCode_Break;
}
