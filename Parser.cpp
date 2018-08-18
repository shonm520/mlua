#include "GramTreeNode.h"
#include "Parser.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

Parser::Parser(vector<string> &filenames)
{
	this->_vtfilenames = filenames;
	_pSyntaxTree = nullptr;
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
			syntaxError(_strCurParserFileName, "identifier", t);
			return false;
		}
	}
	else  {       //期待一个符号,例如",("等
		if (t.lexeme != token.lexeme)  {
			syntaxError(_strCurParserFileName, token.lexeme, t);
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
	string fullname = _strCurParserFileName + "." + name;
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

bool Parser::isBasicType(string type)
{
	if (type == "int" || type == "char" || type == "boolean" || type == "string")  {
		return true;
	}
	return false;
}

void Parser::parse_program()
{
	_pSyntaxTree = parse_chunk_list();
}

Parser::TreeNode *Parser::parse_chunk_list()
{
	TreeNode *p = nullptr;
	TreeNodeList nodeList;
	for (auto filenameIter = _vtfilenames.cbegin(); filenameIter != _vtfilenames.cend(); ++filenameIter) {
		int ret = _scanner.openFile(*filenameIter);
		if (ret != 0) continue;
		auto classNameIter = filenameIter->rbegin();
		int begin = 0;
		while (classNameIter != filenameIter->rend()) {
			if (*classNameIter == '/')
				break;
			begin++;
			++classNameIter;
		}
		_strCurParserFileName = filenameIter->substr(filenameIter->size() - begin, begin - 5);
		_scanner.resetRow();
		TreeNode *q = parse_chunk();
		if (getToken().kind != Scanner::ENDOFFILE)
			cerr << "Syntax Error in class " << _strCurParserFileName << ": unexpected token before EOF " << endl;
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
		if (!node || node->getToken().compare(Scanner::Token_End))  {
			break;
		}
		else if (node->getNodeKind() == SyntaxTreeNodeBase::RETURN_STATEMENT_K)  {
			has_return = true;
		}
		else {
			if (has_return)  {
				syntaxError(_strCurParserFileName, "unexpect statement after return statement", peekToken(true));
				return nullptr;
			}
		}
		listNode.Push(node);
	}
	block_node->addChild(listNode.getHeadNode(), 0);
	return block_node;
}

Parser::TreeNode *Parser::parse_class_var_dec_list()
{
	auto token = peekToken(true);
	TreeNodeList nodeList;
	while (token.lexeme == "static" || token.lexeme == "field") {
		TreeNode *q = parse_class_var_dec();
		nodeList.Push(q);
		token = peekToken(false);
	}
	return nodeList.getHeadNode();
}

Parser::TreeNode *Parser::parse_class_var_dec()
{
	TreeNode *t = new TreeNode(CLASS_VAR_DEC_K);
	Scanner::Token token = peekToken(true);
	if (!eatExpectedToken(token))  {
		return t;
	}
	auto node = new TreeNode;
	node->setLexeme(token.lexeme);
	t->addChild(node, 0);
	t->addChild(parse_type(), 1);
	t->addChild(parse_var_name_list(), 2);
	if (eatExpectedToken(Token(Scanner::SYMBOL, ";")))  {
		return t;
	}
	return t;
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
			pNode = new IdentifierNode(tableName_or_varName);
		}
	}
	else   {
		pNode = new IdentifierNode(tableName_or_varName);
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
			tab_sum = new TableIndexField();
			tab_sum->addChild(table_prefix, 0);
			Scanner::Token table_field = getToken();
			tab_sum->addChild(new IdentifierNode(table_field), 1);
			table_prefix = tab_sum;
			eatExpectedToken(Token(Scanner::SYMBOL, "]"));
		}
		token = peekToken(true);
	}
	return tab_sum;
}

Parser::TreeNode *Parser::parse_type()
{
	TreeNode *t = nullptr;
	Scanner::Token token = getToken();
	if (token.kind == Scanner::ID) {
		t = new TreeNode(CLASS_TYPE_K);
		t->setNodeKind(CLASS_TYPE_K);
		t->setLexeme(token.lexeme);
	}
	else if (token.lexeme == "int" ||
		     token.lexeme == "char" ||
			 token.lexeme == "boolean" ||
			 token.lexeme == "void")  {
		t = new TreeNode(BASIC_TYPE_K);
		t->setLexeme(token.lexeme);
	}
	else {
		syntaxError(_strCurParserFileName, "basic type or class type", token);
		return t;
	}
	return t;
}

Parser::TreeNode *Parser::parse_subroutine_dec_list()
{
	auto token = peekToken(true);
	TreeNodeList nodeList;
	while (token.lexeme == "constructor" ||
		   token.lexeme == "function" || 
		   token.lexeme == "method") {
		TreeNode *q = parse_subroutin_dec();    //解析其中一个函数
		TreeNode::quitSubRoutineZone();
		nodeList.Push(q);
		
		token = peekToken(true);
	}
	return nodeList.getHeadNode();
}

Parser::TreeNode *Parser::parse_subroutin_dec()
{
	TreeNode *t = new SubroutineDecNode();

	Scanner::Token token = getToken();
	if (token.lexeme == "constructor" ||
		token.lexeme == "function" ||
		token.lexeme == "method")  {
		auto node = new TreeNode();
		node->setToken(token);
		t->addChild(node, SubroutineDecNode::Sign);
	}
	else {
		syntaxError(_strCurParserFileName, "constructor or function or method", token);
		return t;
	}
	t->addChild(parse_type(), SubroutineDecNode::Ret);
	token = getToken();
	if (token.kind == Scanner::ID) {
		auto node = new TreeNode();
		node->setToken(token);
		node->setLexeme(getFullName(token.lexeme));
		t->addChild(node, SubroutineDecNode::Name);
	}
	else {
		syntaxError(_strCurParserFileName, "identifile", token);
		return t;
	}

	if (! eatExpectedToken(Token(Scanner::SYMBOL, "(")))  {
		return t;
	}
	t->addChild(parse_params(), SubroutineDecNode::Params);
	if (!eatExpectedToken(Token(Scanner::SYMBOL, ")")))  {
		return t;
	}
	t->addChild(parse_subroutine_body(), SubroutineDecNode::Body);
	t->quitSubRoutineBodyZone();
	return t;
}

Parser::TreeNode *Parser::parse_params()
{
	TreeNode *t = nullptr;
	Scanner::Token token = peekToken(true);
	if (token.lexeme != ")") {
		t = parse_param_list();
	}
	return t;
}

Parser::TreeNode *Parser::parse_param_list()
{
	TreeNode *t = parse_param();
	TreeNodeList nodeList;
	nodeList.Push(t);
	Scanner::Token token = peekToken(true);
	while (token.lexeme == ",") {
		eatExpectedToken(token);
		TreeNode *q = parse_param();
		nodeList.Push(q);
		token = peekToken(true);
	}
	return nodeList.getHeadNode();
}

Parser::TreeNode *Parser::parse_param()
{
	TreeNode *t = new TreeNode(PARAM_K);
	t->addChild(parse_type(), 0);                 //类型
	Scanner::Token token = getToken();
	if (token.kind == Scanner::ID) {
		auto node = new TreeNode;;
		node->setToken(token);
		t->addChild(node, 1);                    //值
	}
	else {
		syntaxError(_strCurParserFileName, "identifier", token);
		return t;
	}
	return t;
}

Parser::TreeNode *Parser::parse_subroutine_body()
{
	_hasRetStatement = false;

	TreeNode *t = new SubroutineBodyNode();

	if (!eatExpectedToken(Token(Scanner::Token_LeftBrace)))  {
		return t;
	}

	parse_statements();

	if (!eatExpectedToken(Token(Scanner::Token_RightBrace)))  {
		return t;
	}

	return t;
}

Parser::TreeNode *Parser::parse_var_dec_list(TreeNodeList& statementNodeList)
{
	TreeNodeList nodeList;
	Scanner::Token token = peekToken(true); 
	bool is_val_dec = false;           //任何语句都可以走到这里来,所以要判断是否是变量声明语句
	if (isBasicType(token.lexeme))  {
		TreeNode *q = parse_var_dec(statementNodeList);
		nodeList.Push(q);
		TreeNode::s_curVarDecType = q->getChildByIndex(VarDecNode::VarDec_Type);     //int a,b 先保存a的声明,后面要给b的
		is_val_dec = true;
	}
	else if (token.kind == Scanner::ID) { // 类变量声明
		token = peekToken(false);
		if (token.kind == Scanner::ID) {
			TreeNode *q = parse_var_dec(statementNodeList);
			nodeList.Push(q);
			TreeNode::s_curVarDecType = q->getChildByIndex(VarDecNode::VarDec_Type);
			is_val_dec = true;
		}
	}

	if (is_val_dec)  {
		token = peekToken(true);
		while (token.lexeme == ",")  {
			eatExpectedToken(Token(Scanner::Token_Comma));
			TreeNode *q = parse_var_dec(statementNodeList);
			nodeList.Push(q);
			token = peekToken(true);
		}
		eatExpectedToken(Token(Scanner::Token_Semicolon));
	}
	TreeNode::s_curVarDecType = nullptr;
	return nodeList.getHeadNode();
}

Parser::TreeNode *Parser::parse_var_dec(TreeNodeList& statementNodeList)
{
	TreeNode *t = new VarDecNode();
	Scanner::Token token;
	TreeNode* node_type = parse_type();                     //形如int a, b, c 解析到b时
	if (TreeNode::s_curVarDecType )  {
		if (TreeNode::s_curVarDecType != node_type)  {
			ungetToken();
			node_type = TreeNode::s_curVarDecType->clone();
		}
	}
	t->addChild(node_type, VarDecNode::VarDec_Type);
	
	token = getToken();
	TreeNode* node_name = new TreeNode();
	node_name->setToken(token);
	t->addChild(node_name, VarDecNode::VarDec_Name);
	token = getToken();
	if (token.lexeme == "=")  {
		ungetToken();
		ungetToken();
		TreeNode* node = parse_assign_statement();
		statementNodeList.Push(node);
	}
	else if (token.lexeme != ";" && token.lexeme != ","){
		syntaxError(_strCurParserFileName, ";", token);
		return t;
	}
	ungetToken();
	return t;
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
	else if (token.compare(Scanner::Token_End))  {
		return new SyntaxTreeNodeBase(token);
	}
	else if (token.compare(Scanner::Token_If))  {
		return parse_if_statement();
	}
	else if (token.compare(Scanner::Token_Return))  {
		TreeNode* rt_smt = parse_return_statement();
		return rt_smt;
	}
	else  {
		if (token.kind != Scanner::ID)  {
			syntaxError(_strCurParserFileName, "a Identifier", token);
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
		syntaxError(_strCurParserFileName, "unexpect token after \'local\'", token);
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
	eatExpectedToken(Token(Scanner::Token_End));

	FunctionStatement* fct_stm = new FunctionStatement();
	fct_stm->addChild(fct_name, FunctionStatement::EFuncName);
	fct_stm->addChild(param_list, FunctionStatement::EFuncParams);
	fct_stm->addChild(fct_body, FunctionStatement::EFuncBody);
	fct_stm->setGlobal(global);
	fct_stm->setAnony(anony);
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

Parser::TreeNode *Parser::parse_statement()
{
	TreeNode *t = nullptr;
	Scanner::Token token = peekToken(true);
	if (token.lexeme == "if") {
		t = parse_if_statement();
		TreeNode::quitCompoundStatmentZone();
	}
	else if (token.lexeme == "while") {
		t = parse_while_statement();
		TreeNode::quitCompoundStatmentZone();
	}
	else if (token.lexeme == "return") {
		t = parse_return_statement();
	}
	else if (token.kind == Scanner::ID) {
		token = peekToken(false);
		if (token.lexeme == "=" || token.lexeme == "[") {
			t = parse_assign_statement();
		}
		else if (token.lexeme == "(" || token.lexeme == ".") {
			t = parse_call_statement(nullptr);
			if (!eatExpectedToken(Token(Scanner::Token_Semicolon)))  {
				return t;
			}
		}
		else {
			syntaxError(_strCurParserFileName, "'=' or '[' or '(' or '.'", token);
			return t;
		}
	}
	else {
		syntaxError(_strCurParserFileName, "identifier", token);
		return t;
	}
	return t;
}

Parser::TreeNode *Parser::parse_assign_statement()
{
	TreeNode *t = new AssignStatement();
	TreeNode* left_val = parse_left_value();
	t->addChild(left_val, AssignStatement::AssignLetf);
	Scanner::Token token = getToken();
	t->addChild(parse_expression(), AssignStatement::AssignRight);
	token = getToken();
	if (token.lexeme != ";" && token.lexeme != ",") {      //赋值语句之后也有可能是,例如int a= 1, b=2;
		syntaxError(_strCurParserFileName, ";", token);
		return t;
	}
	return t;
}

Parser::TreeNode *Parser::parse_left_value()
{
	TreeNode *t = new TreeNode(VAR_K);
	Scanner::Token token = getToken();
	t->setToken(token);
	token = getToken();
	if (token.lexeme == "[") {
		t->setNodeKind(ARRAY_K);
		t->addChild(parse_expression(), 0);
		if (!eatExpectedToken(Token(Scanner::Token_RightSquareBracket)))  {
			return t;
		}
	}
	else if (token.lexeme == "=") {
		ungetToken();
	}
	return t;
}

Parser::TreeNode *Parser::parse_if_statement()
{
	TreeNode *if_stm = new CompondStatement(IF_STATEMENT_K);
	Scanner::Token token = getToken();

	if_stm->addChild(parse_expression(), 0);

	if (!eatExpectedToken(Token(Scanner::Token_Then)))  {
		return if_stm;
	}

	TreeNode* if_blok = parse_statements();
	if_stm->addChild(if_blok, 1);

	token = peekToken(true);
	if (token.compare(Scanner::Token_Else)) {
		eatExpectedToken(Token(Scanner::Token_Else));
		TreeNode* else_blok = parse_statements();
		if_stm->addChild(else_blok, 2);
	}
	return if_stm;
}

Parser::TreeNode *Parser::parse_while_statement()
{
	TreeNode *t = new CompondStatement(WHILE_STATEMENT_K);
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
	assert(eatExpectedToken(Token(Scanner::Token_Assign)));
	TreeNode* node_value = parse_expression();

	assign_node->addChild(node_index, AssignStatement::AssignLetf);
	assign_node->addChild(node_value, AssignStatement::AssignRight);

	assert(eatExpectedToken(Token(Scanner::Token_RightSquareBracket)));
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
		TreeNode *p = new TreeNode;
		p->setNodeKind(COMPARE_K);
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
	while (token.lexeme == "+" || token.lexeme == "-") {
		eatExpectedToken(Token(token));
		OperateStatement::OperateType optype = (token.lexeme == "+") ? OperateStatement::Plus : OperateStatement::Minus;
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
