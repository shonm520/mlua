#include "CodeWriter.h"


CodeWrite::CodeWrite() : 
	num(0)
{
	_instrucions = new Instruction[1000];
}


CodeWrite::~CodeWrite()
{
	delete[] _instrucions;
}

Instruction_* CodeWrite::newInstruction()  {
	Instruction* ins = &_instrucions[num++];
	memset(ins, 0, sizeof(Instruction));
	return ins;
}

void CodeWrite::putInstructions(std::vector<Instruction*>& vtIns) {
	for (int i = 0; i < num; i++)  {
		Instruction* ins = new Instruction(_instrucions[i]);
			vtIns.push_back(ins);
	}
}