#include "GramTreeNode.h"
#include "Parser.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <string>


#define  SafeDelete(p) if(p) delete p; 

Parser::Parser(string str)
{
	_pSyntaxTree = nullptr;
	_scanner.setParseString(true);
	_scanner.setStringCode(str);
	_cirQueue.SetScanner(&_scanner);
}

Parser::Parser(vector<string> &filenames)
{
	_vtFileNames = filenames;
	_pSyntaxTree = nullptr;
	_scanner.setParseString(false);
	_cirQueue.SetScanner(&_scanner);
}

Scanner::Token Parser::getToken()
{
	return _cirQueue.getCurrent();
}

void Parser::ungetToken()
{
	_cirQueue.back(1);
}


bool Parser::eatExpectedToken(Token token, Scanner::Token* pToken)
{
	Scanner::Token t = getToken();
	if (token.kind == Scanner::ID)  {    //期待一个变量标识符
		if (t.kind != Scanner::ID)  {
			syntaxError(_strCurFileName, "identifier", t);
			return false;
		}
	}
	else  {       //期待一个符号,例如",("等
		if (t.lexeme != token.lexeme)  {
			syntaxError(_strCurFileName, token.lexeme, t);
			return false;
		}
	}
	if (pToken)  {
		*pToken = t;
	}
	return true;
}


Scanner::Token Parser::peekToken(bool reset)
{
	return _cirQueue.peekToken(reset);
}

string Parser::getFullName(string name)
{
	string fullname = _strCurFileName + "." + name;
	return fullname;
}

string Parser::getCallerName(string fullName)
{
	auto iter = fullName.cbegin();
	while (iter != fullName.cend()) {
		if (*iter == '.')
			break;
		++iter;
	}
	return string(fullName.cbegin(), iter);
}

string Parser::getFunctionName(string fullName)
{
	auto iter = fullName.cbegin();
	while (iter != fullName.cend()) {
		if (*iter == '.')
			break;
		++iter;
	}
	return string(++iter, fullName.cend());
}


void Parser::parse_program()
{
	_pSyntaxTree = parse_chunk_list();
}

Parser::TreeNode *Parser::parse_chunk_list()
{
	TreeNode *p = nullptr;
	TreeNodeList nodeList;
	for (auto it = _vtFileNames.cbegin(); it != _vtFileNames.cend(); ++it) {
		int ret = _scanner.openFile(*it);
		if (ret != 0) continue;
		auto classNameIter = it->rbegin();
		int begin = 0;
		while (classNameIter != it->rend()) {
			if (*classNameIter == '/')
				break;
			begin++;
			++classNameIter;
		}
		_strCurFileName = *it;
		_scanner.resetRow();
		TreeNode *q = parse_chunk();
		if (getToken().kind != Scanner::ENDOFFILE)
			cerr << "Syntax Error in file " << _strCurFileName << ": unexpected token before EOF " << endl;
		nodeList.Push(q);
		_scanner.closeFile();
	}
	return nodeList.getHeadNode();
}

Parser::TreeNode *Parser::parse_chunk()
{
	ChunkNode* chunk_node = new ChunkNode();
	chunk_node->addChild(parse_block(), 0);
	return chunk_node;
}

Parser::TreeNode *Parser::parse_block()
{
	BlockNode* block_node = new BlockNode();
	TreeNodeList listNode;
	bool has_return = false;
	while (true)  {
		if (peekToken(true).kind == Scanner::ENDOFFILE)  {
			break;
		}
		TreeNode* node = parse_statements();
		if (!node) break;
		if (node->getToken().compare(Scanner::Token_End))  {
			eatExpectedToken(Token(Scanner::Token_End));
			break;
		}
		else if (node->getNodeKind() == SyntaxTreeNodeBase::RETURN_STATEMENT_K)  {
			has_return = true;
		}
		else {
			if (has_return)  {
				syntaxError(_strCurFileName, "unexpect statement after return statement", peekToken(true));
				return nullptr;
			}
		}
		listNode.Push(node);
	}
	block_node->addChild(listNode.getHeadNode(), 0);
	return block_node;
}

Parser::TreeNode *Parser::parse_var_name_list()
{
	TreeNodeList nodeList;
	Token token = peekToken(true);
	while (token.lexeme == ",") {
		eatExpectedToken(token);
		Scanner::Token tableName_or_varName = getToken();
		token = peekToken(true);
		if (token.compare("[") || token.compare("."))  {
			SyntaxTreeNodeBase* pNode = parse_access_table_Field(new IdentifierNode(tableName_or_varName));
			nodeList.Push(pNode);
		}
		else  {
			IdentifierNode* pNode = new IdentifierNode(tableName_or_varName);
			nodeList.Push(pNode);
		}
		token = peekToken(true);
	}
	return nodeList.getHeadNode();
}

Parser::TreeNode * Parser::parse_var_name(bool bList)      //解析变量名,有带表名 t.a, t[a]等 是否解析后面的变量,解析函数的实参时就不需要,因为函数参数有可能是string,他是不能作变量的
{
	SyntaxTreeNodeBase* pNode = nullptr;
	Scanner::Token tableName_or_varName = getToken();      //如果不是单个变量就是table名了
	Scanner::Token token = peekToken(true);
	if (token.compare("[") || token.compare("."))  {
		pNode = parse_access_table_Field(new IdentifierNode(tableName_or_varName));
		token = peekToken(true);
		if (token.compare(","))  {
			if (bList)  {
				TreeNode* node_rest = parse_var_name_list();
				pNode->setNextNode(node_rest);
			}
		}
	}
	else if (token.compare(","))  {
		if (bList)  {
			pNode = new IdentifierNode(tableName_or_varName);
			TreeNode* node_rest = parse_var_name_list();
			pNode->setNextNode(node_rest);
		}
		else  {
			if (tableName_or_varName.kind == Scanner::INT)  {
				pNode = new Terminator(tableName_or_varName, Terminator::TERM_NUMBER);
			}
			else  {
				pNode = new IdentifierNode(tableName_or_varName);
			}
		}
	}
	else   {
		if (tableName_or_varName.kind == Scanner::INT)  {
			pNode = new Terminator(tableName_or_varName, Terminator::TERM_NUMBER);
		}
		else  {
			pNode = new IdentifierNode(tableName_or_varName);
		}
	}
	return pNode;
}

Parser::TreeNode *Parser::parse_access_table_Field(SyntaxTreeNodeBase* table_prefix)
{
	Token token = peekToken(true);
	SyntaxTreeNodeBase* tab_sum = nullptr;
	while (token.compare(".") || token.compare("["))  {
		eatExpectedToken(token);
		if (token.compare("."))  {
			tab_sum = new TabMemberAccessor();
			tab_sum->addChild(table_prefix, 0);
			Scanner::Token table_field = getToken();
			tab_sum->addChild(new IdentifierNode(table_field), 1);
			table_prefix = tab_sum;
		}
		else  {
			tab_sum = new TabIndexAccessor();
			tab_sum->addChild(table_prefix, 0);
			Scanner::Token table_field = getToken();
			TreeNode* filed = nullptr;
			if (table_field.kind == Scanner::INT)  {
				filed = new Terminator(table_field, Terminator::TERM_NUMBER);
			}
			else if (table_field.kind == Scanner::STRING)  {
				filed = new Terminator(table_field, Terminator::TERM_STRING);
			}
			else if (table_field.kind == Scanner::ID)  {
				filed = new IdentifierNode(table_field);
			}
			else  {
				syntaxError(_strCurFileName, "id or number ,string", table_field);
			}
			tab_sum->addChild( filed, 1);
			table_prefix = tab_sum;
			eatExpectedToken(Token(Scanner::SYMBOL, "]"));
		}
		token = peekToken(true);
	}
	return tab_sum;
}

Parser::TreeNode *Parser::parse_statements()    //解析一个块里的所有语句,包括声明赋值调用,子语句
{
	Scanner::Token token = peekToken(true);
	if (token.compare("local"))  {
		return parse_local_statement();
	}
	else if (token.compare(Scanner::Token_Function))  {
		return parse_function_statement(false);
	}
	else if (token.compare(Scanner::Token_If))  {
		return parse_if_statement();
	}
	else if (token.compare(Scanner::Token_Return))  {
		TreeNode* rt_smt = parse_return_statement();
		return rt_smt;
	}
	else if (token.compare(Scanner::Token_For))  {
		token = peekToken(false);
		token = peekToken(false);
		if (token.compare(Scanner::Token_Assign))  {              //数值型for循环
			return parse_numeric_for_statement();
		}
		else if (token.compare(Scanner::Token_Comma))  {          //泛型for循环
			return parse_generic_for_statement();
		}
		else {
			return nullptr;
		}
	}
	else  {
		if (token.compare(Scanner::Token_End))  {            //块结束
			eatExpectedToken(Token(Scanner::Token_End));
			return nullptr;
		}
		else if (token.compare(Scanner::Token_Else) ||       //if的一个分支结束
				 token.compare(Scanner::Token_ElseIf))  {
			return nullptr;
		}
		else if (token.compare(Scanner::Token_Break))  {
			getToken();
			return new BreakStatement();
		}
		if (token.kind != Scanner::ID)  {
			syntaxError(_strCurFileName, "a Identifier", token);
			return nullptr;
		}
		SyntaxTreeNodeBase* node = parse_var_name(true);
		Scanner::Token token = peekToken(true);
		if (token.compare("("))  {  //f()
			return parse_call_statement(node);
		}
		else if (token.compare("="))  {
			return parse_assign_statement_lua(node);
		}
		else  {
			return nullptr;
		}
	}
}

Parser::TreeNode *Parser::parse_local_nameList()
{
	SyntaxTreeNodeBase* left_vals = parse_var_name(true);
	Scanner::Token token = peekToken(true);

	if (token.compare("="))  {
		TreeNode *t = new LocalNameListStatement();
		t->addChild(left_vals, LocalNameListStatement::NameList);
		eatExpectedToken(Token(Scanner::Token_Assign));
		TreeNode* right_exps = parse_expression_list();
		t->addChild(right_exps, LocalNameListStatement::ExpList);
		return t;
	}
	else  {                 //形如local a   右面为空
		TreeNode *asn_stm = new AssignStatement();
		asn_stm->addChild(left_vals, AssignStatement::AssignLetf);
		return asn_stm;
	}
}

Parser::TreeNode *Parser::parse_local_statement()
{
	eatExpectedToken(Token(Scanner::Token_Local));
	Token token = peekToken(true);
	if (token.compare(Scanner::Token_Function))  {
		return parse_function_statement(false, true);
	}
	else if (token.kind == Scanner::ID)  {
		return parse_local_nameList();
	}
	else  {
		syntaxError(_strCurFileName, "unexpect token after \'local\'", token);
		return nullptr;
	}
	return nullptr;
}

Parser::TreeNode *Parser::parse_function_statement(bool anony, bool global)
{
	eatExpectedToken(Token(Scanner::Token_Function));
	SyntaxTreeNodeBase* fct_name = nullptr;
	if (! anony)  {     //不是匿名函数
		fct_name = parse_var_name(true);
	}
	
	eatExpectedToken(Token(Scanner::Token_LeftRoundBracket));
	Token token = peekToken(true);
	SyntaxTreeNodeBase* param_list = nullptr;
	while (! token.compare(Scanner::Token_RightRoundBracket))  {
		param_list = parse_var_name(true);
		token = peekToken(true);
	}
	eatExpectedToken(Token(Scanner::Token_RightRoundBracket));
	SyntaxTreeNodeBase* fct_body = parse_block();

	FunctionStatement* fct_stm = new FunctionStatement();
	fct_stm->addChild(fct_name, FunctionStatement::EFuncName);
	fct_stm->addChild(param_list, FunctionStatement::EFuncParams);
	fct_stm->addChild(fct_body, FunctionStatement::EFuncBody);
	fct_stm->setGlobal(global);
	return fct_stm;
}

Parser::TreeNode *Parser::parse_assign_statement_lua(SyntaxTreeNodeBase* left_vals)    //解析形如a, b = 1, 2
{
	TreeNode *t = new AssignStatement();
	t->addChild(left_vals, AssignStatement::AssignLetf);
	eatExpectedToken(Token(Scanner::Token_Assign));
	TreeNode* right_exps = parse_expression_list();
	t->addChild(right_exps, AssignStatement::AssignRight);
	return t;
}

Parser::TreeNode *Parser::parse_if_statement()
{
	TreeNode* if_stm = new IfStatement();
	Scanner::Token token = getToken();         //可以是if 也可以是elseif
	if_stm->addChild(parse_expression(), IfStatement::ECompare);
	if (!eatExpectedToken(Token(Scanner::Token_Then)))  {
		return if_stm;
	}

	TreeNode* if_blok = parse_block();
	if_stm->addChild(if_blok, IfStatement::EIf);
	token = peekToken(true);
	if (token.compare(Scanner::Token_Else)) {
		eatExpectedToken(Token(Scanner::Token_Else));
		TreeNode* else_blok = parse_block();
		if_stm->addChild(else_blok, IfStatement::EElseOrEnd);
	}
	else if (token.compare(Scanner::Token_ElseIf))  {
		TreeNode* elseIf = parse_if_statement();
		TreeNode* block = new BlockNode();
		block->addChild(elseIf, 0);
		if_stm->addChild(block, IfStatement::EElseOrEnd);
	}
	return if_stm;
}

Parser::TreeNode *Parser::parse_while_statement()
{
	TreeNode *t = new TreeNode(WHILE_STATEMENT_K);
	Scanner::Token token = getToken();
	
	t->addChild(parse_expression(), 0);

	if (!eatExpectedToken(Token(Scanner::Token_Then)))  {
		return t;
	}
	t->addChild(parse_statements(), 1);
	if (!eatExpectedToken(Token(Scanner::Token_End)))  {
		return t;
	}
	return t;
}

Parser::TreeNode *Parser::parse_return_statement()
{
	TreeNode *rt_smt = new ReturnStatement();
	Scanner::Token token;
	eatExpectedToken(Token(Scanner::Token_Return), &token);
	rt_smt->setToken(token);
	rt_smt->addChild(parse_expression_list(), 0);

	return rt_smt;
}

Parser::TreeNode *Parser::parse_call_statement(SyntaxTreeNodeBase* caller_node)
{
	NormalCallFunciton *call_statement = new NormalCallFunciton();
	call_statement->addChild(caller_node, 0);
	Token token = getToken();
	if (token.lexeme == "(") {
		TreeNode* exp_list = parse_expressions();
		if (!eatExpectedToken(Token(Scanner::Token_RightRoundBracket)))  {
			return call_statement;
		}
		call_statement->addChild(exp_list, 1);
	}
	
	return call_statement;
}

Parser::TreeNode *Parser::parse_expressions()    //貌似只有解析函数实参数调用列表时才用到
{
	TreeNode *t = nullptr;
	Scanner::Token token = peekToken(true);
	if (token.lexeme == ")") {
		return t;
	}
	else {
		t = parse_expression_list();
		return t;
	}
}

Parser::TreeNode *Parser::parse_expression_list()
{
	TreeNode *t = parse_expression();
	TreeNodeList nodeList;
	nodeList.Push(t);
	Scanner::Token token = peekToken(true);
	while (token.compare(",")) {
		eatExpectedToken(Token(token));
		TreeNode *q = parse_expression();
		nodeList.Push(q);
		token = peekToken(true);
	}
	return nodeList.getHeadNode();
}

Parser::TreeNode *Parser::parse_expression()
{
	Scanner::Token token = peekToken(true);
	if (token.compare("{"))  {                            //构造表{}
		return parse_table_constructor();
	}
	if (token.compare(Scanner::Token_Function))  {        //构造匿名函数
		return parse_function_statement(true);
	}
	else  {
		TreeNode *t = parse_bool_expression();
		Scanner::Token token = peekToken(true);
		while (token.lexeme == "&" || token.lexeme == "|") {
			eatExpectedToken(token);
			TreeNode *p = new TreeNode(BOOL_EXPRESSION_K);
			p->setToken(token);
			p->addChild(t, 0);
			t = p;
			t->addChild(parse_bool_expression(), 1);
			token = peekToken(true);
		}
		return t;
	}
}

Parser::TreeNode *Parser::parse_table_constructor()
{
	if (eatExpectedToken(Token(Scanner::Token_LeftBrace)))  {
		TableDefine* node_table = new TableDefine();
		Token token = peekToken(true);
		while (! token.compare("}"))  {
			if (token.compare("["))  {
				node_table->push_filed(parse_build_table_indexField());      //t = {[a]}
			}
			else if (token.kind == Scanner::ID && peekToken(false).compare("="))  {
				node_table->push_filed(parse_build_table_nameField());       // t = {a = 1}
			}
			else  {
				node_table->push_filed(parse_build_table_arrayField());      //t = {a,b}
			}
			token = peekToken(true);
			if (! token.compare("}"))  {
				eatExpectedToken(Token(Scanner::Token_Comma));
				token = peekToken(true);
			}
		}
		eatExpectedToken(Token(Scanner::Token_RightBrace));
		return node_table;
	}
	else  {
		return nullptr;
	}
}

Parser::TreeNode *Parser::parse_build_table_indexField()
{
	TableIndexField* assign_node = new TableIndexField();
	assert(eatExpectedToken(Token(Scanner::Token_LeftSquareBracket)));
	TreeNode* node_index = parse_var_name(false);
	assert(eatExpectedToken(Token(Scanner::Token_RightSquareBracket)));
	assert(eatExpectedToken(Token(Scanner::Token_Assign)));
	TreeNode* node_value = parse_expression();

	assign_node->addChild(node_index, AssignStatement::AssignLetf);
	assign_node->addChild(node_value, AssignStatement::AssignRight);

	return assign_node;
}

Parser::TreeNode *Parser::parse_build_table_nameField()
{
	TableNameField* assign_node = new TableNameField();
	TreeNode* node_name = parse_var_name(false);
	assert(eatExpectedToken(Token(Scanner::Token_Assign)));
	TreeNode* node_value = parse_expression();

	assign_node->addChild(node_name, AssignStatement::AssignLetf);
	assign_node->addChild(node_value, AssignStatement::AssignRight);

	return assign_node;
}

Parser::TreeNode *Parser::parse_build_table_arrayField()
{
	TableArrayFiled* arr_node = new TableArrayFiled();
	TreeNode* node_name = parse_var_name(false);
	arr_node->addChild(node_name, 0);
	return arr_node;
}

Parser::TreeNode *Parser::parse_bool_expression()
{
	TreeNode *t = parse_additive_expression();
	Scanner::Token token = peekToken(true);
	if (token.lexeme == "<=" || token.lexeme == ">=" || token.lexeme == "=="
		|| token.lexeme == "<" || token.lexeme == ">" || token.lexeme == "!=") {
		eatExpectedToken(token);
		TreeNode *p = new CompareStatement();
		p->setToken(token);
		p->addChild(t, 0);
		t = p;
		t->addChild(parse_additive_expression(), 1);
	}
	return t;
}

Parser::TreeNode *Parser::parse_additive_expression()
{
	TreeNode *t = parse_term();
	Scanner::Token token = peekToken(true);
	while (token.lexeme == "+" || 
		   token.lexeme == "-" ||
		   token.lexeme == "..") {
		eatExpectedToken(Token(token));
		OperateStatement::OperateType optype = (token.lexeme == "-") ? OperateStatement::Minus : OperateStatement::Plus;
		TreeNode *p = new OperateStatement(optype);
		p->setToken(token);
		p->addChild(t, 0);
		t = p;
		p->addChild(parse_term(), 1);
		token = peekToken(true);
	}
	return t;
}

Parser::TreeNode *Parser::parse_term()
{
	TreeNode *t = parse_factor();
	Scanner::Token token = peekToken(true);
	while (token.lexeme == "*" || token.lexeme == "/") {
		eatExpectedToken(token);
		OperateStatement::OperateType optype = (token.lexeme == "*") ? OperateStatement::Multi : OperateStatement::Div;
		TreeNode *p = new OperateStatement(optype);
		p->setToken(token);
		p->addChild(t, 0);
		t = p;
		p->addChild(parse_factor(), 1);
		token = peekToken(true);
	}
	return t;
}

Parser::TreeNode *Parser::parse_factor()
{
	TreeNode *t = nullptr;
	Scanner::Token token = peekToken(true);
	if (token.lexeme == "-") {
		eatExpectedToken(token);
		t = new UnaryExpression(token);
		t->setToken(token);
		t->addChild(parse_positive_factor(), 0);
	}
	else {
		t = parse_positive_factor();
	}
	return t;
}

Parser::TreeNode *Parser::parse_positive_factor()
{
	TreeNode *t = nullptr;
	Scanner::Token token = peekToken(true);
	if (token.lexeme == "~") {
		eatExpectedToken(token);
		t = new TreeNode(BOOL_EXPRESSION_K);
		t->setToken(token);
		t->addChild(parse_not_factor(), 0);
	}
	else {
		t = parse_not_factor();
	}
	return t;
}

Parser::TreeNode *Parser::parse_not_factor()
{
	TreeNode *t = nullptr;
	Scanner::Token token = getToken();
	if (token.lexeme == "(") {
		t = parse_expression();
		if (!eatExpectedToken(Token(Scanner::Token_RightRoundBracket)))  {
			return t;
		}
	}
	else if (token.kind == Scanner::INT) {
		t = new Terminator(token, Terminator::TERM_NUMBER);
	}
	else if (token.kind == Scanner::CHAR) {
		t = new TreeNode(CHAR_CONST_K);
		t->setToken(token);
	}
	else if (token.kind == Scanner::STRING) {
		t = new Terminator(token, Terminator::TERM_STRING);
	}
	else if (token.lexeme == "true") {
		t = new Terminator(token, Terminator::TERM_TRUE);
	}
	else if (token.lexeme == "false")  {
		t = new Terminator(token, Terminator::TERM_FALSE);
	}
	else if (token.lexeme == "nil") {
		t = new Terminator(token, Terminator::TERM_NIL);
	}
	else if (token.kind == Scanner::ID) {
		t = new TreeNode(VAR_K);
		ungetToken();
		t = parse_var_name(false);
		t->setToken(token);
		token = peekToken(true);
		if (token.lexeme == "[") {
			eatExpectedToken(Token(Scanner::Token_LeftSquareBracket));
			TreeNode *p = parse_expression();
			t->addChild(p, 0);
			if (!eatExpectedToken(Token(Scanner::Token_RightSquareBracket)))  {
				return t;
			}
			t->setNodeKind(ARRAY_K);
		}
		else if (token.lexeme == "(" || token.lexeme == ".") {
			t = parse_call_expression(t);
		}
	}
	return t;
}

Parser::TreeNode *Parser::parse_call_expression(SyntaxTreeNodeBase* caller)
{
	return parse_call_statement(caller);
}

Parser::TreeNode* Parser::parse_numeric_for_statement()     //for Name = exp , exp [, exp] do block end 
{
	NumericForStatement* nodeFor = nullptr;
	do   {
		eatExpectedToken(Token(Scanner::Token_For));
		Scanner::Token token = getToken();
		if (token.kind != Scanner::ID)  {
			syntaxError(_strCurFileName, "identifier", token);
			break;
		}
		TreeNode *start = new IdentifierNode(token);
		if (!eatExpectedToken(Token(Scanner::Token_Assign)))  {
			SafeDelete(start);
			break;
		}

		TreeNode* exp = parse_expression();
		if (!eatExpectedToken(Token(Scanner::Token_Comma)))  {
			SafeDelete(start);
			SafeDelete(exp);
			break;
		}
		TreeNode* assin = new LocalNameListStatement();
		assin->addChild(start, 0);
		assin->addChild(exp, 1);

		TreeNode* end = parse_expression();
		TreeNode* step = nullptr;
		
		token = peekToken(true);
		if (token.compare(Scanner::Token_Comma))  {      //i = 1,2,-1 do
			eatExpectedToken(token);
			step = parse_expression();
		}
		if (!eatExpectedToken(Token(Scanner::Token_Do)))  {
			SafeDelete(start);
			SafeDelete(exp);
			SafeDelete(step);
			SafeDelete(assin);
			break;
		}

		TreeNode* block = parse_block();
		nodeFor = new NumericForStatement();
		nodeFor->addChild(assin, 0);
		nodeFor->addChild(end, 1);
		nodeFor->addChild(step, 2);
		nodeFor->addChild(block, 3);

	} while (0);
	return nodeFor;
}

Parser::TreeNode* Parser::parse_generic_for_statement()     //for namelist in explist do block end 
{
	GenericForStatement* nodeFor = nullptr;
	do   {
		eatExpectedToken(Token(Scanner::Token_For));

		TreeNode* namelist = parse_var_name(true);
		eatExpectedToken(Token(Scanner::Token_In));
		TreeNode* explist = parse_expression_list();

		eatExpectedToken(Token(Scanner::Token_Do));

		TreeNode* block = parse_block();
		nodeFor = new GenericForStatement();
		nodeFor->addChild(namelist, 0);
		nodeFor->addChild(explist, 1);
		nodeFor->addChild(block, 2);

	} while (0);
	return nodeFor;
}

void Parser::print()
{
	printSyntaxTree(_pSyntaxTree);
}

void Parser::printSyntaxTree(TreeNode *tree, int dep)
{
	while (tree != nullptr) {
		cout << "\n";
		for (int i = 0; i < dep; i++)
			cout << "  ";
		cout << "| ";
		switch (tree->getNodeKind()) {
		case CLASS_K:
			cout << "class" ;
			break;
		case CLASS_VAR_DEC_K:
			cout << "class_var_dec" ;
			break;
		case SUBROUTINE_DEC_K:
			cout << "subroutine_dec" ;
			break;
		case BASIC_TYPE_K:
			cout << "basic_type " << tree->getLexeme() ;
			break;
		case CLASS_TYPE_K:
			cout << "class_type " << tree->getLexeme();
			break;
		case PARAM_K:
			cout << "param" ;
			break;
		case VAR_DEC_K:
			cout << "var_dec" ;
			break;
		case ARRAY_K:
			cout << "array" ;
			break;
		case VAR_K:
			cout << "var" ;
			break;
		case IF_STATEMENT_K:
			cout << "if_statement" ;
			break;
		case WHILE_STATEMENT_K:
			cout << "while_statement" ;
			break;
		case RETURN_STATEMENT_K:
			cout << "return_statement" ;
			break;
		case CALL_STATEMENT_K:
			cout << "call_statement" ;
			break;
		case CALL_EXPRESSION_K:
			cout << "call_expression";
			break;
		case BOOL_EXPRESSION_K:
			cout << "bool_expression " << tree->getLexeme();
			break;
		case COMPARE_K:
			cout << "compare " << tree->getLexeme();
			break;
		case OPERATION_K:
			cout << "operation " << tree->getLexeme();
			break;
		case BOOL_K:
			cout << "bool" ;
			break;
		case ASSIGN_K:
			cout << "assign" ;
			break;
		case SUBROUTINE_BODY_K:
			cout << "subroutine_body" ;
			break;
		}

		for (int i = 0; i < TreeNode::Child_Num_Const; i++)  {
			printSyntaxTree(tree->getChildByIndex(i), dep + 2);
		}
		tree = tree->getNextNode();
	}
}

Parser::TreeNode *Parser::getSyntaxTree()
{
	return _pSyntaxTree;
}
