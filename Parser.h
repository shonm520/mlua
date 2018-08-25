#ifndef _PARSER_H
#define _PARSER_H

#include "Scanner.h"
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <queue>
#include "Error.h"
#include "GramTreeNode.h"

using namespace std;


class Parser
{
public:
    enum NodeKind {
        None, CLASS_K, CLASS_VAR_DEC_K, SUBROUTINE_DEC_K, BASIC_TYPE_K, CLASS_TYPE_K, NULL_K,
        PARAM_K, VAR_DEC_K, ARRAY_K, VAR_K, IF_STATEMENT_K, WHILE_STATEMENT_K, CALL_EXPRESSION_K,
        RETURN_STATEMENT_K, CALL_STATEMENT_K, BOOL_EXPRESSION_K,FUNCTION_CALL_K,CONSTRUCTOR_CALL_K,
		COMPARE_K, OPERATION_K, BOOL_K, ASSIGN_K, SUBROUTINE_BODY_K, BOOL_CONST_K, NEGATIVE_K,
        INT_CONST_K, CHAR_CONST_K, STRING_CONST_K, KEY_WORD_CONST, THIS_K
    };

	friend class BaseLib;

 	typedef SyntaxTreeNodeBase TreeNode;

	typedef Scanner::Token Token;

	struct CirQueue
	{
		CirQueue()  {
			_pScanner = nullptr;
			_top = 0;
			_cur = -1;
			_peek = -1;
		}
		int _peek;
		int _top, _cur;
		Scanner* _pScanner;
		static const int Cap = 20;
		Scanner::Token _member[Cap];
		void SetScanner(Scanner* ps)  {
			_pScanner = ps;
		}

		static void increase(int& n, int step = 1)  {
			n = (n + Cap + step) % Cap;
		}

		static bool isEqual(int cur, int top)  {
			return cur == top;
		}

		Scanner::Token getCurrent()  {
			increase(_cur);
			if (isEqual(_cur, _top))  {
				Scanner::Token token = _pScanner->nextToken();
				_member[_top] = token;
				increase(_top);
			}
			resetPeek();
			return _member[_cur];
		}

		Scanner::Token peekToken(bool reset)  {
			if (reset)  {
				resetPeek();
			}
			increase(_peek);
			if (isEqual(_peek, _top))  {
				Scanner::Token token = _pScanner->nextToken();
				_member[_top] = token;
				increase(_top);
			}
			return _member[_peek];
		}

		void resetPeek()  {
			_peek = _cur;
		}
		void back(int step = 1)  {
			increase(_cur, step * -1);
		}
	};
private:
    vector<string> _vtFileNames;
	bool _parseString;
    string _strCurFileName;
    TreeNode *_pSyntaxTree;
    Scanner _scanner;
    bool _hasRetStatement;                              // 要保证每个函数都有return语句, 即使返回值为void

    Scanner::Token getToken();                          // 从缓冲区中取出一个token
    void ungetToken();                        // 把上一次取出的token放入到缓冲区中
	bool eatExpectedToken(Token token, Scanner::Token* pToken = nullptr);
	Scanner::Token peekToken(bool reset);

	CirQueue _cirQueue;
    string getFullName(string name);                    // 返回

    TreeNode * parse_chunk_list();
    TreeNode * parse_chunk();
	TreeNode * parse_var_name(bool bList);
	TreeNode * parse_var_name_list();
	TreeNode * parse_statements();
	TreeNode * parse_assign_statement_lua(SyntaxTreeNodeBase* left_vals);
    TreeNode * parse_statement();
    TreeNode * parse_assign_statement();
    TreeNode * parse_left_value();
    TreeNode * parse_if_statement();
    TreeNode * parse_while_statement();
    TreeNode * parse_return_statement();
	TreeNode * parse_call_statement(SyntaxTreeNodeBase* caller);
    TreeNode * parse_expressions();
    TreeNode * parse_expression_list();
    TreeNode * parse_expression();
    TreeNode * parse_bool_expression();
    TreeNode * parse_additive_expression();
    TreeNode * parse_term();
    TreeNode * parse_factor();
    TreeNode * parse_positive_factor();
    TreeNode * parse_not_factor();
	TreeNode * parse_call_expression(SyntaxTreeNodeBase* caller);

	TreeNode * parse_function_statement(bool anony, bool global = false);

	TreeNode * parse_table_constructor();
	TreeNode * parse_build_table_indexField();
	TreeNode * parse_build_table_nameField();
	TreeNode * parse_build_table_arrayField();

	TreeNode * parse_access_table_Field(SyntaxTreeNodeBase* table_prefix);

	TreeNode * parse_block();
	TreeNode * parse_local_statement();
	TreeNode * parse_local_nameList();

	TreeNode * parse_for_statement();
	TreeNode * parse_numeric_for_statement();
	TreeNode * parse_generic_for_statement();

    void printSyntaxTree(TreeNode *tree, int dep = 1);

public:
	Parser();
    Parser(vector<string> & filenames);
    bool hasError();
    TreeNode *getSyntaxTree();
    void print();
    void parse_program();
    static string getCallerName(string fullName);
    static string getFunctionName(string fullName);
};

#endif
