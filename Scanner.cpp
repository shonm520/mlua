#include "Scanner.h"
#include <cstring>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <iterator>

using namespace std;

Scanner::Scanner()
{
    _row = 0;
    _nBufferPos = 0;
	_parseString = false;
    initKeyWords();
    initSymbols();
}

void Scanner::resetRow()
{
    _row = 0;
}

int Scanner::openFile(string filename)
{
    string suffix = filename.substr(filename.size() - 4, 4);
    if (suffix != ".lua")  {
        cerr <<  "file '"<<filename << "' is not a standard lua  file" << endl;
		return -1;
    }
    _fIn.open(filename);
    if (_fIn.fail())  {
        cerr << "file " << filename << " does not exit" << endl;
		return -1;
    }
	return 0;
}

void Scanner::closeFile()
{
    _fIn.close();
}

void Scanner::initKeyWords()
{
    m_vtkeyWords.insert("and");
    m_vtkeyWords.insert("break");
    m_vtkeyWords.insert("function");
    m_vtkeyWords.insert("do");
    m_vtkeyWords.insert("end");
    m_vtkeyWords.insert("then");
    m_vtkeyWords.insert("in");
    m_vtkeyWords.insert("local");
    m_vtkeyWords.insert("nil");
    m_vtkeyWords.insert("not");
	m_vtkeyWords.insert("or");
	m_vtkeyWords.insert("repeat");
	m_vtkeyWords.insert("until");
    m_vtkeyWords.insert("true");
    m_vtkeyWords.insert("false");
    m_vtkeyWords.insert("for");
    m_vtkeyWords.insert("if");
    m_vtkeyWords.insert("else");
	m_vtkeyWords.insert("elseif");
    m_vtkeyWords.insert("while");
    m_vtkeyWords.insert("return");
}

void Scanner::initSymbols()
{
    m_vtStrSymbols.insert("{");
    m_vtStrSymbols.insert("}");
    m_vtStrSymbols.insert("(");
    m_vtStrSymbols.insert(")");
    m_vtStrSymbols.insert("[");
    m_vtStrSymbols.insert("]");
    m_vtStrSymbols.insert(".");
    m_vtStrSymbols.insert(",");
    m_vtStrSymbols.insert(";");
    m_vtStrSymbols.insert("+");
    m_vtStrSymbols.insert("-");
    m_vtStrSymbols.insert("*");
    m_vtStrSymbols.insert("/");
    m_vtStrSymbols.insert("&");
    m_vtStrSymbols.insert("|");
    m_vtStrSymbols.insert("~");
    m_vtStrSymbols.insert("<");
    m_vtStrSymbols.insert(">");
    m_vtStrSymbols.insert("=");
    m_vtStrSymbols.insert(">=");
    m_vtStrSymbols.insert("<=");
    m_vtStrSymbols.insert("==");
    m_vtStrSymbols.insert("!=");
}

string Scanner::detailType2Lexeme(TokenType_Detail detail)
{
	string lexeme;
	switch (detail)
	{
	case Scanner::Token_And:
		lexeme = "and";
		break;
	case Scanner::Token_Break:
		lexeme = "break";
		break;
	case Scanner::Token_Do:
		lexeme = "do";
		break;
	case Scanner::Token_Else:
		lexeme = "else";
		break;
	case Scanner::Token_ElseIf:
		lexeme = "elseif";
		break;
	case Scanner::Token_End:
		lexeme = "end";
		break;
	case Scanner::Token_False:
		break;
	case Scanner::Token_For:
		lexeme = "for";
		break;
	case Scanner::Token_Function:
		lexeme = "function";
		break;
	case Scanner::Token_If:
		lexeme = "if";
		break;
	case Scanner::Token_In:
		lexeme = "in";
		break;
	case Scanner::Token_Local:
		lexeme = "local";
		break;
	case Scanner::Token_Nil:
		lexeme = "nil";
		break;
	case Scanner::Token_Not:
		lexeme = "not";
		break;
	case Scanner::Token_Or:
		lexeme = "or";
		break;
	case Scanner::Token_Repeat:
		lexeme = "repeat";
		break;
	case Scanner::Token_Return:
		lexeme = "return";
		break;
	case Scanner::Token_Then:
		lexeme = "then";
		break;
	case Scanner::Token_True:
		lexeme = "true";
		break;
	case Scanner::Token_Until:
		lexeme = "until";
		break;
	case Scanner::Token_While:
		lexeme = "while";
		break;
	case Scanner::Token_Id:
		break;
	case Scanner::Token_String:
		break;
	case Scanner::Token_Number:
		break;
	case Scanner::Token_Equal:
		lexeme = "==";
		break;
	case Scanner::Token_NotEqual:
		lexeme = "~=";
		break;
	case Scanner::Token_LessEqual:
		lexeme = "<=";
		break;
	case Scanner::Token_GreaterEqual:
		lexeme = ">=";
		break;
	case Scanner::Token_Concat:
		break;
	case Scanner::Token_VarArg:
		break;
	case Scanner::Token_EOF:
		break;
	case Scanner::Token_LeftRoundBracket:
		lexeme = "(";
		break;
	case Scanner::Token_RightRoundBracket:
		lexeme = ")";
		break;
	case Scanner::Token_LeftSquareBracket:
		lexeme = "[";
		break;
	case Scanner::Token_RightSquareBracket:
		lexeme = "]";
		break;
	case Scanner::Token_LeftBrace:
		lexeme = "{";
		break;
	case Scanner::Token_RightBrace:
		lexeme = "}";
		break;
	case Scanner::Token_Comma:
		lexeme = ",";
		break;
	case Scanner::Token_Semicolon:
		lexeme = ";";
		break;
	case Scanner::Token_Assign:
		lexeme = "=";
		break;
	default:
		break;
	}
	return lexeme;
}

Scanner::TokenType Scanner::searchReserved(string &s)
{
    if (m_vtkeyWords.find(s) != m_vtkeyWords.end())
        return KEY_WORD;
    else
        return ID;
}

char Scanner::nextChar()
{
	if (_parseString)  {
		_strLineBuffer = _stringCode;
		if (_nBufferPos > _stringCode.size())   {
			return EOF;
		}
		else  {
			return _strLineBuffer[_nBufferPos++];
		}
	}
	else  {
		if (_nBufferPos >= _strLineBuffer.size())  {
			_row++;
			getline(_fIn, _strLineBuffer);
			_strLineBuffer += '\n';
			if (!_fIn.fail())  {
				_nBufferPos = 0;
				return _strLineBuffer[_nBufferPos++];
			}
			else  {
				return EOF;
			}
		}
		else  {
			return _strLineBuffer[_nBufferPos++];
		}
	}
}

void Scanner::rollBack()
{
    assert(_nBufferPos > 0);
    _nBufferPos--;
}

Scanner::State Scanner::procStartState(char ch, Token& token)
{
	State state = STATE_START;
	if (ch == ' ' || ch == '\t' || ch == '\n');
	else if (isalpha(ch) || ch == '_')  {
		state = STATE_ID;		// 进入标识符状态
		token.kind = ID;
		token.lexeme += ch;
	}
	else if (isdigit(ch))  {
		state = STATE_INT;		// 进入整数状态
		token.kind = INT;
		token.lexeme += ch;
	}
	else if (m_vtStrSymbols.find({ ch }) != m_vtStrSymbols.end())  {
		state = STATE_SYMBOL;
		token.kind = SYMBOL;
		token.lexeme += ch;
	}
	else if (ch == '"')  {
		state = STATE_STRING;	// 进入字符串状态
		token.kind = STRING;
	}
	else if (ch == '\'')  {		// 进入单字符状态
		state = STATE_CHAR;
		token.kind = CHAR;
	}
	else	{											// 其它非法字符
		state = STATE_ERROR;
		token.kind = ERROR;
		token.lexeme += ch;
	}
	token.row = _row;
	return state;
}

bool Scanner::procIntState(char ch, Token& token)
{
	if (isdigit(ch))  {
		token.lexeme += ch;
	}
	else  {
		rollBack();
		return true;
	}
	return false;
}

bool Scanner::procIdentityState(char ch, Token& token)
{
	if (isalpha(ch) || isdigit(ch) || ch == '_')  {
		token.lexeme += ch;
	}
	else  {
		rollBack();
		return true;
	}
	return false;
}

void Scanner::procStringState(char ch, Token& token, Scanner::State& state)
{
	if (state == STATE_STRING)  {
		if (ch == '"')  {            //结束解析字符串
			state = STATE_DONE;
		}
		else if (ch == '\\')  {      //开始或结束子字符串
			state = STATE_S_STRING;
			token.lexeme += ch;
		}
		else  {
			token.lexeme += ch;
		}
	}
	else if (state == STATE_S_STRING)  {   //进入子字符串和解析字符串一样
		state = STATE_STRING;
		if (ch == '\'')                    //去掉\ "AB\n\'ab\'CD"   ==> "AB\n'ab'CD"   其实不用去掉任何'\'这里分为STATE_STRING和STATE_S_STRING，只是划分""结束的界限,"\""依旧是合法的，不至于遇到"就表明字符串结束了
			token.lexeme.pop_back();
		token.lexeme += ch;
	}
}

void Scanner::procCharState(char ch, Token& token, Scanner::State& state)
{
	bool match = false;
	if (state == STATE_CHAR)  {                 //字符开始'
		if (ch != '\\' && ch != '\'')  {        //字符'A'开始
			state = STATE_CHAR_A;
			token.lexeme += ch;
			match = true;
		}
		else if (ch == '\\')  {
			state = STATE_CHAR_B;
			token.lexeme += ch;
			match = true;
		}
	}
	else if (state == STATE_CHAR_A)  {       //字符'A'结束
		if (ch == '\'')  {
			state = STATE_DONE;
			match = true;
		}
	}
	else if (state == STATE_CHAR_B)  {      //形如字符'\a'
		if (ch == 'a' || ch == 'b' || ch == 'f' || ch == 'n' || ch == 'r' || ch == 't' ||
			ch == 'v' || ch == '\\' || ch == '?' || ch == '\'' || ch == '"')  {
			state = STATE_CHAR_C;
			token.lexeme += ch;
			match = true;
		}
	}
	else if (state == STATE_CHAR_C)  {      //形如字符'\a'结束
		if (ch == '\'')  {
			state = STATE_DONE;
			match = true;
		}
	}
	if (!match)  {
		state = STATE_ERROR;
		token.kind = ERROR;
		string tmp = "'";
		tmp.append(token.lexeme);
		tmp += ch;
		token.lexeme = tmp;
	}
}

void Scanner::procSymbolState(char ch, Token& token, Scanner::State& state)
{
	bool match = false;
	if (token.lexeme == "/")  {
		if (ch == '*')  {
			state = STATE_COMMENT;
			token.lexeme.clear();
			match = true;
		}
		else if (ch == '/')  {
			state = STATE_START;
			_nBufferPos = _strLineBuffer.length();
			token.lexeme.clear();
			match = true;
		}
	}
	else if (token.lexeme == "<")  {
		if (ch == '=')  {
			token.lexeme += ch;
			state = STATE_DONE;
			match = true;
		}
	}
	else if (token.lexeme == "=")  {
		if (ch == '=')  {
			token.lexeme += ch;
			state = STATE_DONE;
			match = true;
		}
	}
	else if (token.lexeme == "!")  {
		if (ch == '=')  {
			token.lexeme += ch;
			state = STATE_DONE;
			match = true;
		}
	}
	else if (token.lexeme == "&")  {
		if (ch == '&')  {
			token.lexeme += ch;
			state = STATE_DONE;
			match = true;
		}
	}
	else if (token.lexeme == "|")  {
		if (ch == '|')  {
			token.lexeme += ch;
			state = STATE_DONE;
			match = true;
		}
	}
	if (!match)  {
		rollBack();
		state = STATE_DONE;
	}
}

void Scanner::procCommentState(char ch, Token& token, Scanner::State& state)
{
	if (state == STATE_COMMENT)  {
		if (ch == '*')                     //注释状态下，遇到*,说明有可能是注释结束*/
			state = STATE_P_COMMENT;
	}
	else if (state == STATE_P_COMMENT)  {
		if (ch == '/')                     //注释真的结束了
			state = STATE_START;
		else
			state = STATE_COMMENT;         //不然的话还是注释例如 /*this is * */ 
	}
}

Scanner::Token Scanner::nextToken()
{
    Token token;
    unsigned tokenStringIndex = 0;
    State state = STATE_START;
    while (state != STATE_DONE)  {
        char ch = nextChar();
        if (ch == EOF)  {
            token.kind = ENDOFFILE;
            break;
        }
		switch (state)  {
        case STATE_START:										// 开始状态
			state = procStartState(ch, token);
            break;

        case STATE_INT:											// 整数状态
			if (procIntState(ch, token)) state = STATE_DONE;
            break;

        case STATE_ID:											// 标识符状态
			if (procIdentityState(ch, token)) state = STATE_DONE;
            break;

		case STATE_SYMBOL:
			procSymbolState(ch, token, state);
			break;

		case STATE_CHAR:										// 字符状态
		case STATE_CHAR_A:
		case STATE_CHAR_B:
		case STATE_CHAR_C:
			procCharState(ch, token, state);
			break;

        case STATE_STRING:										// 字符串状态
        case STATE_S_STRING:
			procStringState(ch, token, state);
            break;

        case STATE_COMMENT:								    	// 注释状态
		case STATE_P_COMMENT:
			procCommentState(ch, token, state);
            break;

		case STATE_ERROR:										// 错误状态
			if (ch == ' ' || ch == '\n' || ch == '\t')
				state = STATE_DONE;
			else
				token.lexeme += ch;
			break;
        }
        if (state == STATE_DONE && token.kind == ID)
            token.kind = searchReserved(token.lexeme);          //有可能是关键字
    }
    return token;
}
