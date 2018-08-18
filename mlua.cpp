#include "Scanner.h"
#include "Parser.h"
#include "State.h"
#include "CodeGenerate.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2)  {
        cerr << "usage: " << argv[0] << " <filename, filename ... >" << endl;
        exit(-1);
    }
    ifstream fin;
    vector<string> filenames;
    for (int i = 1; i < argc; i++)  {
        fin.open(argv[i]);
        if (fin.fail())  {
            cerr << "file '" << argv[i] << "' not exist!";
            exit(-1);
        }
        filenames.push_back(argv[i]);
        fin.close();
    }

    Parser parser(filenames);
    parser.parse_program();
    if (!hasError())  {
		State state;
		CodeGenerate(parser.getSyntaxTree(), &state);
    }

	getchar();
    return 0;
}
