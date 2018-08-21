#include "CodeWriter.h"
#include "Function.h"


CodeWrite::CodeWrite() :
	_insSet(new InstructionSet(InstructionSet::Medium))
{
	
}


CodeWrite::~CodeWrite()
{
	if (_insSet)  {
		_insSet->clearInstructions();
	}
}

Instruction* CodeWrite::newInstruction()  {
	return _insSet->newInstruction();
}

InstructionSet* CodeWrite::fetchInstructionSet()
{
	InstructionSet* temp = _insSet;
	_insSet = nullptr;
	return temp;
}

InstructionValue* CodeWrite::fetchInstructionVal()
{
	InstructionValue* val = new InstructionValue();
	val->setInstructionSet(_insSet);
	_insSet = nullptr;
	return val;
}
