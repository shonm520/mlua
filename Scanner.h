#ifndef _Scanner_H_
#define _Scanner_H_

#include <string>
#include <fstream>
#include <deque>
#include <set>

using namespace std;

class Scanner
{
private:
    enum State	// 转移图中的状态
    {
		STATE_UNREADY = -1,
        STATE_START,		// 开始状态
        STATE_ID,			// 标识符状态
        STATE_INT,			// 整型数状态
        STATE_CHAR,			// 字符状态		
        STATE_CHAR_A,
        STATE_CHAR_B,
        STATE_CHAR_C,
        STATE_FLOAT,		// 浮点数状态
        STATE_D_FLOAT,		// 接近带小数点的浮点数状态
        STATE_E_FLOAT,		// 接近科学技术法的浮点数状态
        STATE_STRING,		// 字符串状态
        STATE_S_STRING,		// 含有转移字符的字符串
        STATE_SYMBOL, 
        STATE_COMMENT,   	// 注释状态
        STATE_P_COMMENT,	// 快要结束注释状态
        STATE_DONE,			// 结束状态
        STATE_ERROR			// 错误状态
    };

public:
    set<string> m_vtkeyWords;
    set<string> m_vtStrSymbols;
    enum TokenType
    {      
        KEY_WORD,
        ID,				// 标识符
        INT,			// 整型数字
        BOOL,			// 布尔类型
        CHAR,			// 字符
        STRING,			// 字符串
        SYMBOL,         // 合法的符号
        NONE,		    // 无类型
        ERROR,		    // 错误
        ENDOFFILE	    // 文件结束
    };

	enum TokenType_Detail
	{
		Token_And = 256, Token_Break, Token_Do, Token_Else, Token_Elseif, Token_End,
		Token_False, Token_For, Token_Function, Token_If, Token_In,
		Token_Local, Token_Nil, Token_Not, Token_Or, Token_Repeat,
		Token_Return, Token_Then, Token_True, Token_Until, Token_While,
		Token_Id, Token_String, Token_Number,
		Token_Equal, Token_NotEqual, Token_LessEqual, Token_GreaterEqual,
		Token_Concat, Token_VarArg, Token_EOF,
		Token_LeftRoundBracket,Token_RightRoundBracket,
		Token_LeftSquareBracket, Token_RightSquareBracket,
		Token_LeftBrace, Token_RightBrace,
		Token_Comma, Token_Semicolon,
		Token_Assign
	};
    struct Token
    {
        TokenType kind;				// Token的类型
        string lexeme;				// Token的值
        unsigned row;	   	        // 当前行

		bool compare(TokenType_Detail td)  {
			return (this->lexeme == detailType2Lexeme(td));
		}
		bool compare(string lexeme)  {
			return (this->lexeme == lexeme);
		}
		explicit Token() = default;
		Token(const Token& t)  {
			kind = t.kind;
			lexeme = t.lexeme;
			row = t.row;
		}
		Token(TokenType_Detail type_det)  {
			kind = SYMBOL;
			lexeme = detailType2Lexeme(type_det);
		}
		Token(TokenType k, string l = "")  {
			kind = k;
			lexeme = l;
		}
    };
	
    void initKeyWords();
    void initSymbols();
private:
    string _strLineBuffer;					// 缓冲行, 保存源程序中的一行数据
    unsigned _nBufferPos;					// 缓冲行的指针
    unsigned _row;					     	// 保存当前缓冲行在源程序中的行号
    ifstream _fIn;					    	// 源程序文件的输入流对象
    char nextChar();					    // 返回缓冲区中的下一个字符
    void rollBack();					    // 回滚缓冲区
    TokenType searchReserved(string &s);	// 查找关键字

	static string detailType2Lexeme(TokenType_Detail detail);

	State procStartState(char ch, Token& token);
	bool procIntState(char ch, Token& token);
	bool procIdentityState(char ch, Token& token);
	void procStringState(char ch, Token& token, Scanner::State& state);
	void procCharState(char ch, Token& token, Scanner::State& state);
	void procSymbolState(char ch, Token& token, Scanner::State& state);
	void procCommentState(char ch, Token& token, Scanner::State& state);
public:
    Scanner();
    int openFile(string filename);
    void closeFile();
    Token nextToken();					  // 返回下一个Token
    void resetRow();
};

#endif
