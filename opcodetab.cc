/*  Andrew Black, Andy Risser, Darren  Barnachea, Kent Vo, Shayan Rastegarpour
 *  masc0836
 *  Team Kansas
 *  prog2
 *  CS530, Spring 2016
 */

#include<map>
#include<string>
#include "opcodetab.h"
#include "opcode_error_exception.h"

using namespace std;

opcodetab::opcodetab(){
	for(int i = 0; i < totalopcodes; i++)
		opcodemap[opcodes[i]] = pair<string, int>( hexadecimals[i], formatsizes[i] );
}

string opcodetab::get_machine_code(string tmp_opcode) {
	bool type_four = false;
	format_opcode(tmp_opcode, type_four);
	
	// Looks for opcode in map, throws error if not found
	if(opcodemap.find(tmp_opcode) == opcodemap.end())
		throw opcode_error_exception("ERROR: Opcode does not exist.");

	// returns appropriate hexadecimal code
	return opcodemap.find(tmp_opcode)->second.first;
}

int opcodetab::get_instruction_size(string tmp_opcode) {
	bool type_four = false;
	format_opcode(tmp_opcode, type_four);
	
	//searches for code in opcodemap and returns the appropriate size, otherwise throws error
	if(opcodemap.find(tmp_opcode)==opcodemap.end())
		throw opcode_error_exception("ERROR: Opcode does not exist.");

	//returns 4 if size 3 found for the inputed code, otherwise throw error
	if(type_four) {
		if(opcodemap.find(tmp_opcode)->second.second == 3)
			return 4;
		
		throw opcode_error_exception("ERROR: Format 4 of opcode does not exist.");
	}
		
	return opcodemap.find(tmp_opcode)->second.second;
}

//changes input to upper case
void opcodetab::format_opcode(string &tmp_opcode, bool &type_four){
	if(tmp_opcode[0] == '+') {
		tmp_opcode = tmp_opcode.erase(0,1);
		type_four = true;
	}
		
	transform( tmp_opcode.begin(), tmp_opcode.end(), tmp_opcode.begin(), ::toupper );
}
