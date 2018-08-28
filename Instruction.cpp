#include "Instruction.h"
#include <string.h>    //for memset in linux

Instruction::Instruction()
{

}



InstructionSet::InstructionSet(Volum v) :
	_num(0),
	_instructions(nullptr)
{
	if (v == Large)  {
		_instructions = new Instruction[1000];
	}
	else if (v == Medium)  {
		_instructions = new Instruction[500];
	}
	else if (v == Small)  {
		_instructions = new Instruction[100];
	}
	else if (v == None)  {
		_instructions = 0;
	}
}


std::vector<Instruction*> InstructionSet::toVtInstructions() {
	_opcodes.clear();
	for (int i = 0; i < _num; i++)  {
		_opcodes.push_back(&_instructions[i]);
	}
	return _opcodes;
}


Instruction* InstructionSet::newInstruction()
{
	Instruction* ins = &_instructions[_num++];
	memset(ins, 0, sizeof(Instruction));
	return ins;
}


void InstructionSet::clearInstructions()
{
	delete[] _instructions;
	_num = 0;
}