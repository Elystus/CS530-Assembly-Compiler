/*  Andrew Black, Andy Risser, Darren  Barnachea, Kent Vo, Shayan Rastegarpour
    masc0836
    Team Kansas
    prog4
    CS530, Spring 2016
 */

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include <fstream> 
#include <cstdio>

#include "sicxe_asm.h"
#include "file_parser.h"
#include "file_parse_exception.h"
#include "opcodetab.h"
#include "opcode_error_exception.h"
#include "symtab.h"
#include "symtab_exception.h"


using namespace std;

// SICXE_ASM Class Entry Point
sicxe_asm::sicxe_asm(string file) {
	base_address = 0;
	set_base = false;
	filename = file;
	
	try { 
		// Get Contents of file
		file_parser file_tokenized(filename);
		file_tokenized.read_file(); 
		
		// Assembler First Pass
		first_pass(file_tokenized);
		
		// Assembler Second Pass
		second_pass();
		
		// Write to .lis file
		write_to_file();
		generate_obj();
	
	// Error Catchers
	} catch (opcode_error_exception e) {
        	cout << e.getMessage() << endl;
        	exit(1);
    	} catch (file_parse_exception e) {
        	cout << e.getMessage() << endl;
        	exit(1);
    	} catch (symtab_exception e) {
        	cout << e.getMessage() << endl;
        	exit(1);
    	}
}

sicxe_asm::~sicxe_asm() {}

/*
 * SECOND PASS
 * Assembler Second Pass Code
 */
void sicxe_asm::second_pass() {
	for (unsigned int i = 0; i < first_pass_vector.size(); i++) {
		unsigned int line = first_pass_vector[i].line;
		string addr  = first_pass_vector[i].addr;
		string label  = first_pass_vector[i].label;
		string opcode  = first_pass_vector[i].opcode;
		string operand  = first_pass_vector[i].operand;
		
		string machine = "";
		if(directive_check(opcode)) {
			machine = to_upper(directive_helper_sp(first_pass_vector[i]));
		} else if(!opcode.empty()) {
			int format = opcode_index.get_instruction_size(opcode);

			// Format 1
			if(format == 1) 
				machine = to_upper(helper_sp_format1(first_pass_vector[i]));
			
			// Format 2
			else if (format == 2) 
				machine = to_upper(helper_sp_format2(first_pass_vector[i]));
			
			// Format 3
			else if (format == 3) 
				machine = to_upper(helper_sp_format3(first_pass_vector[i]));
			
			// Format 4
			else if (format == 4) 
				machine = to_upper(helper_sp_format4(first_pass_vector[i]));
			
			// FORMAT NOT FOUND
			else 
				throw_error("INVALID FORMAT TYPE", line);
		}
		
		second_pass_row row;
		row.line = line;
		row.addr = addr;
		row.label = label;
		row.opcode  = opcode;
		row.operand = operand;
		row.machine = machine;
		
		// Push row struct to Second Pass Container
		second_pass_vector.push_back(row);
	}
}

unsigned int sicxe_asm::nixbpe_generator(bool n, bool i, bool x, bool b, bool p, bool e, first_pass_row curr_row) {
	unsigned int bit_n = 0x20;
	unsigned int bit_i  = 0x10;
	unsigned int bit_x = 0x08;
	unsigned int bit_b = 0x04;
	unsigned int bit_p = 0x02;
	unsigned int bit_e = 0x01;
	
	if(!n && !i)
		throw_error("Invalid NIXBPE combination!", curr_row.line);
	if(b && p)
		throw_error("Invalid NIXBPE combination!", curr_row.line);
	if((b||p) && e)
		throw_error("Invalid NIXBPE combination!", curr_row.line);
	if(!(n && i) && x)
		throw_error("Invalid NIXBPE combination!", curr_row.line);
		
	// nixbpe holder variable
	unsigned int nixbpe = 0x00;
	if(n)
		nixbpe += bit_n;
	if(i)
		nixbpe += bit_i;
	if(x)
		nixbpe += bit_x;
	if(b)
		nixbpe += bit_b;
	if(p)
		nixbpe += bit_p;
	if(e)
		nixbpe += bit_e;
		
	return nixbpe;
}

// DIRECTIVE HELPER
string sicxe_asm::directive_helper_sp(first_pass_row curr_row) {
	string machine = "";
	string opcd = to_upper(curr_row.opcode);

	if(opcd == "START")
		return "";

	else if(opcd == "END")
		return "";

	else if(opcd == "BYTE")
		return directive_helper_sp_byte(curr_row);

	else if(opcd == "WORD")
		return directive_helper_sp_word(curr_row);

	else if(opcd == "RESB")
		return "";

	else if(opcd == "RESW")
		return "";

	else if(opcd == "BASE") {
		directive_helper_sp_base(curr_row);
		return "";
	}

	else if(opcd == "NOBASE"){
		base_address = 0;
		return "";
	}

	else if(opcd == "EQU") {
		if(is_hex(curr_row.operand))
			equ_mod_fail.push_back(curr_row.label);
		return "";
	}

	else
		return "";
}
	
void sicxe_asm::directive_helper_sp_base(first_pass_row curr_row) {
	unsigned int addr = get_label(curr_row.operand, curr_row);
	
	if(addr > 0xFFFFF)
		throw_error("Out of address range", curr_row.line);
	
	// Set base address
	base_address = addr;
}

string sicxe_asm::directive_helper_sp_byte(first_pass_row curr_row) {
	string operand = curr_row.operand;
	string machine = "";
	
	if(to_upper(operand).at(0) == 'C') {
		operand = operand.erase(0,2);
		operand = operand.erase(operand.size() - 1, operand.size());
		
		for(int i = 0 ; i < operand.size() ; i++) {
			machine += hex_to_string((int) operand[i], 2);
		}
	} else if(to_upper(operand).at(0) == 'X') {
		operand = operand.erase(0,2);
		operand = operand.erase(operand.size() - 1, operand.size());
		
		machine = operand;
	} else {
		throw_error("Incorrect Operand Format", curr_row.line);
	}
	
	return machine;
}

string sicxe_asm::directive_helper_sp_word(first_pass_row curr_row) {
	string operand = curr_row.operand;
	string machine = "";
	
	unsigned int conv;
	
	if(operand.at(0) == '$') {
		operand = operand.erase(0,1);
		
		if(!is_hex(operand) || operand.size() > 6)
			throw_error("Invalid Hexadecimal value || Address out of bounds",  curr_row.line);
		
		conv = string_to_hex(operand);
	} else if(is_num(operand)) {
		conv = string_to_int(operand);
	} else {
		throw_error("Invalid operand for directive WORD",  curr_row.line);
	}
	
	if(conv > 0xFFFFFF)
		throw_error("Value out of bounds",  curr_row.line);
		
	machine = hex_to_string(conv, 6);
	return machine;
}

// FORMAT TYPE HELPERS
string sicxe_asm::helper_sp_format1(first_pass_row curr_row) {
	unsigned int line = curr_row.line;
	string opcode = to_upper(curr_row.opcode);
	string operand = curr_row.operand;

	if(!operand.empty())
		throw_error("Format 1 ~ opcode does not support operand",  line);

	return opcode_index.get_machine_code(opcode);
}
string sicxe_asm::helper_sp_format2(first_pass_row curr_row) {
	string tmp_operand = curr_row.operand;

	if(tmp_operand.empty())
		throw_error("No Operand Passed", curr_row.line); 

	string oprd_first  = "";
	string oprd_second = "";
	string machine = "";
	unsigned int tmp_int;

	string delim = ",";
	int offset = tmp_operand.find_first_of(delim);

	machine += opcode_index.get_machine_code(curr_row.opcode);	// Get machine code for OPCODE

	// No comma detected
	if(offset == string::npos) {
		string operand = tmp_operand;

		if(operand.at(0) == '$') {
			operand = operand.erase(0,1);

			if(operand.empty() || !is_hex(operand) || operand.size() > 6)
				throw_error("Invalid Operand", curr_row.line);

			operand = int_to_string(string_to_hex(operand));
		}

		if((to_upper(curr_row.opcode) == "SVC") && is_num(operand)) {
			tmp_int = string_to_int(operand);

			if(tmp_int < 0 || tmp_int > 15)
				throw_error("Invalid Operand", curr_row.line);

			machine += hex_to_string(tmp_int, 1);
		} else if ((to_upper(curr_row.opcode) == "CLEAR" || to_upper(curr_row.opcode) == "TIXR") && !is_num(operand)){
			machine += get_register(operand, curr_row);
		} else {
			throw_error("Invalid Operand and Opcode combination", curr_row.line);
		}

		machine += "0";
	} else if(!(to_upper(curr_row.opcode) == "CLEAR" || to_upper(curr_row.opcode) == "TIXR" || to_upper(curr_row.opcode) == "SVC")) {
		// Separate Operands
		oprd_first = tmp_operand.substr(0,offset);
		oprd_second = tmp_operand.substr((offset));
		oprd_second.erase(0,1); // Remove the comma;

		if(oprd_second.empty())
			throw_error("Invalid Operand", curr_row.line);

		if(oprd_second.at(0) == '$') {
			oprd_second= oprd_second.erase(0,1);

			if(oprd_second.empty() || !is_hex(oprd_second))
				throw_error("Invalid Operand", curr_row.line);

			oprd_second = int_to_string(string_to_hex(oprd_second));
		}

		if((to_upper(curr_row.opcode) == "SHIFTR" || to_upper(curr_row.opcode) == "SHIFTL") && !(!is_num(oprd_first) && is_num(oprd_second)))
			throw_error("Invalid Operand and Opcode combination", curr_row.line);
		else 
			machine += get_register(oprd_first, curr_row);


		if(is_num(oprd_second)) {
			tmp_int = string_to_int(oprd_second) - 1;

			if(tmp_int < 0 || tmp_int > 15) {
				throw_error("Invalid Operand", curr_row.line);
			} if(to_upper(curr_row.opcode) != "SHIFTR" && to_upper(curr_row.opcode) != "SHIFTL") {
				throw_error("Invalid Operand and Opcode combination", curr_row.line);
			}
			machine += hex_to_string_nofill(tmp_int);
		} else {
			machine += get_register(oprd_second, curr_row);
		}
	} else {
		throw_error("Invalid Operand and Opcode Combination", curr_row.line);
	}

	return machine;
}

string sicxe_asm::helper_sp_format3(first_pass_row curr_row) {
	bool n = true;
	bool i = true;
	bool x = false;
	bool b = false;
	bool p = false;
	bool e = false;
	
	string operand = curr_row.operand;
	int pc_offset = 0;
	unsigned int base_offset;
	
	string machine_opcode = opcode_index.get_machine_code(curr_row.opcode);
	unsigned int nixbpe = nixbpe_generator(n,i,x,b,p,e,curr_row);	// Creates default nixbpe
	unsigned int machine_tmp = (string_to_hex(machine_opcode) * 0x10) + nixbpe;
	string machine = "";
	string machine_operand = "";
	
	// Special Case (RSUB)
	if(to_upper(curr_row.opcode) == "RSUB") 
		return hex_to_string(machine_tmp, 3) + "000"; 
		
	if(operand.empty())
		throw_error("This Opcode requires an Operand", curr_row.line);
		
	if(operand.at(0) == '#') {
		operand = operand.substr(1);
		
		if(operand.empty())
			throw_error("Invalid Operand", curr_row.line);

		if(operand.at(0) != '$' && is_num(operand)) {
			unsigned int range_check = string_to_int(operand);
			if(range_check > 0xFFF)
				throw_error("Supplied value out of range", curr_row.line);
			
			operand = int_to_string(range_check);
		}
		
		n = false;
	} else if (operand.at(0) == '@') {
		operand = operand.substr(1);
		
		i = false;
	}
		
	string x_delim = ",";
	if(operand.find_first_of(x_delim) != string::npos) {
		string before_delim = operand.substr(0, operand.find_first_of(x_delim));
		string after_delim  = operand.substr(operand.find_first_of(x_delim));
		after_delim = after_delim.erase(0,1);
		
		operand = before_delim;
		
		// 3rd argument, throw error
		if(after_delim.find_first_of(x_delim) != string::npos)
			throw_error("Invalid Operand format", curr_row.line);
			
		if(to_upper(after_delim) == "X") {
			x = true;
			operand = before_delim;
		} else {
			throw_error("Invalid Operand format", curr_row.line);
		} 
	}
	
	if(!is_num(operand) && operand.at(0) != '$') {
		// unsigned int label_addr = string_to_hex(symbol_table.get_addr(operand));
		unsigned int label_addr = get_label(operand, curr_row);
		pc_offset = relative_pc(curr_row, label_addr);
		
		if(!relative_pc_check(pc_offset)) {
			base_offset = relative_base(curr_row, label_addr);
			
			if(!relative_base_check(base_offset))
				throw_error("Supplied value out of range", curr_row.line);
			
			machine_operand = hex_to_string(base_offset, 3);
			b = true;
		} else {
			machine_operand = hex_to_string(pc_offset, 3);
			p = true;
		}
	} else {
		if(operand.at(0) == '$') {
			operand = operand.substr(1);
		
			if(operand.empty())
				throw_error("Invalid Operand", curr_row.line);

			if(!is_hex(operand) || operand.size() > 6)
				throw_error("Invalid Hexadecimal value", curr_row.line);
		} else {
			operand = hex_to_string(string_to_int(operand), 3);
		}

		if(string_to_hex(operand) > 0xFFF)
			throw_error("Supplied value out of range", curr_row.line);
			
		operand = hex_to_string(string_to_hex(operand), 3);
		machine_operand = operand;
	}
	
	nixbpe 		= nixbpe_generator(n,i,x,b,p,e, curr_row);
	machine_tmp 	= (string_to_hex(machine_opcode) * 0x10) + nixbpe;
	machine		= hex_to_string(machine_tmp, 3) + machine_operand; 
	
	if(to_upper(curr_row.opcode) == "LDB")
		set_base = true;
	return machine;
}

string sicxe_asm::helper_sp_format4(first_pass_row curr_row) {
	bool n = true;
	bool i = true;
	bool x = false;
	bool b = false;
	bool p = false;
	bool e = true;
	
	string operand = curr_row.operand;
	
	string machine_opcode = opcode_index.get_machine_code(curr_row.opcode);
	unsigned int nixbpe = nixbpe_generator(n,i,x,b,p,e, curr_row);	// Creates default nixbpe
	unsigned int machine_tmp = (string_to_hex(machine_opcode) * 0x10) + nixbpe;
	string machine = "";
	string machine_operand = "";
	
	
	if(to_upper(curr_row.opcode) == "RSUB") 
		return hex_to_string(machine_tmp, 3) + "00000"; 
		
	if(operand.empty())
		throw_error("This OPCODE requires an Operand", curr_row.line);

	if(operand.at(0) == '#') {
		operand = operand.substr(1);
		
		if(operand.empty())
			throw_error("Invalid Operand", curr_row.line);

		if(operand.at(0) != '$' && is_num(operand)) {
			unsigned int range_check = string_to_int(operand);
			if(range_check > 0xFFFFF)
				throw_error("Supplied value out of range", curr_row.line);
			
			operand = int_to_string(range_check);
		}
		
		n = false;
	}  else if (operand.at(0) == '@') {
		operand = operand.substr(1);
		
		i = false;
	} 

	string x_delim = ",";
	if(operand.find_first_of(x_delim) != string::npos) {
		string before_delim = operand.substr(0, operand.find_first_of(x_delim));
		string after_delim  = operand.substr(operand.find_first_of(x_delim));
		after_delim = after_delim.erase(0,1);
		
		operand = before_delim;
		
		// 3rd argument, throw error
		if(after_delim.find_first_of(x_delim) != string::npos)
			throw_error("Invalid Operand", curr_row.line);
			
		if(to_upper(after_delim) == "X") {
			x = true;
			operand = before_delim;
		} else {
			throw_error("Invalid Operand", curr_row.line);
		} 
	}
	
	
	if(!is_num(operand) && operand.at(0) != '$') {
		unsigned int label_addr = get_label(operand, curr_row);
		machine_operand = hex_to_string(label_addr, 5);
	} else {
		
		if(operand.at(0) == '$') {
			operand = operand.substr(1);
		
			if(operand.empty())
				throw_error("Invalid Operand", curr_row.line);
			if(!is_hex(operand))
				throw_error("Invalid Hexadecimal value", curr_row.line);
		} else {
			operand = hex_to_string(string_to_int(operand), 5);
		}
		
		if(string_to_hex(operand) > 0xFFFFF)
			throw_error("Supplied value out of range", curr_row.line);
			
		operand = hex_to_string(string_to_hex(operand), 5);
		machine_operand = operand;
	}
	
	if(to_upper(curr_row.opcode) == "+LDB")
		set_base = true;

	nixbpe 		= nixbpe_generator(n,i,x,b,p,e, curr_row);
	machine_tmp 	= (string_to_hex(machine_opcode) * 0x10) + nixbpe;
	machine		= hex_to_string(machine_tmp, 3) + machine_operand; 
	return machine;
}

int sicxe_asm::relative_pc(first_pass_row curr_row, unsigned int label_addr) {
	int curr_addr = ((int) string_to_hex(curr_row.addr)) + 3;
	
	int offset = ((int) label_addr) - curr_addr;
	return offset;
}

bool sicxe_asm::relative_pc_check(int offset) {
	if(offset < 2047 && offset > -2048)
		return true;
	else
		return false;
}

unsigned int sicxe_asm::relative_base(first_pass_row curr_row, unsigned int label_addr) {
	if(!base_address || !set_base) 
		throw_error("Base not set", curr_row.line);
	if(label_addr < base_address) {
		throw_error("Address out of bounds", curr_row.line);
	}
		
	unsigned int offset = label_addr - base_address;
	return offset;
}

bool sicxe_asm::relative_base_check(unsigned int offset) {
	if(offset < 0xFFF)
		return true;
	else
		return false;
}

string sicxe_asm::get_register(string reg, first_pass_row curr_row) {
	reg = to_upper(reg);
	if(reg == "A")
		return "0";
	else if(reg == "X")
		return "1";	
	else if(reg == "L")
		return "2";
	else if(reg == "B")
		return "3";
	else if(reg == "S")
		return "4";
	else if(reg == "T")
		return "5";
	else if(reg == "F")
		return "6";
	else if(reg == "PC")	
		throw_error("PC Register is not available to programmer", curr_row.line);
	else if(reg == "SW")
		throw_error("SW Register is not available to programmer", curr_row.line);	
	else 
		throw_error("Register does not exist", curr_row.line);	

	return "";
}

void sicxe_asm::generate_obj() {
	// File Stream
	ofstream file;
	stringstream in_stream;
	
	// Remove file extension and append .lis
	size_t last_index = filename.find_last_of("."); 
	string tmp_filename = filename.substr(0, last_index); 
	tmp_filename = tmp_filename + ".obj";
	
	file.open( tmp_filename.c_str() , ios::out );
	if (!file)
		throw_error("Could not create object file");

	// Generate Header Record
	string h_record = "H";
	program_name.resize(6,' ');
	h_record += program_name;
	h_record += hex_to_string(program_start, 6);
	h_record += hex_to_string((string_to_hex(second_pass_vector.back().addr) - string_to_hex(second_pass_vector.front().addr)), 6);

	file << to_upper(h_record) << endl;

	vector<string> mod_record;
	string tmp_mod = "";
	string t_record = "";
	string t_record_start = "T" + hex_to_string(program_start, 6);
	string t_record_column_string = "";
	int t_record_column = 0;
	// Generate Text Records
	for (unsigned int i = 0; i < second_pass_vector.size(); i++) {
		second_pass_row tmp_row = second_pass_vector[i];

		if(t_record_start.empty()) {
			t_record_start = "T" + hex_to_string(string_to_hex(tmp_row.addr),6);
		}
		
		// Start new Text Record if columns exceed 69
		if((t_record_column + tmp_row.machine.length()) > 60) {
			file 	<< to_upper(t_record_start)
				<< hex_to_string((t_record_column / 2),2)
				<< t_record 
				<< endl;

			t_record = tmp_row.machine;
			t_record_start = "T" + hex_to_string(string_to_hex(tmp_row.addr),6);
			t_record_column = tmp_row.machine.length();
		} 

		else {
			t_record += tmp_row.machine; // Tack machine code to end of Text Record
			t_record_column += tmp_row.machine.length(); // Modify column length
		}
		
		// If RESB or RESW start a new Text Record (Address Change)
		if ((to_upper(tmp_row.opcode) == "RESB" || to_upper(tmp_row.opcode) == "RESW")) {
			// Write only if machine code pushed to record
			if(!t_record.empty()) 
				file 	<< to_upper(t_record_start)
					<< hex_to_string((t_record_column / 2),2)
					<< t_record 
					<< endl;
		
			t_record_start = t_record = "";
			t_record_column = 0;
		}
	}
	
	if(!t_record.empty()) {
		file 	<< to_upper(t_record_start)
			<< hex_to_string((t_record_column / 2),2)
			<< t_record 
			<< endl;
	}

	// Generate Modification Records
	string tmp_addr = "";
	for (unsigned int i = 0; i < second_pass_vector.size(); i++) {
		second_pass_row tmp_row = second_pass_vector[i];

		if(tmp_row.opcode.empty() && tmp_row.operand.empty())
			continue;
		if(tmp_row.opcode.at(0) != '+')
			continue;
		if(tmp_row.operand.at(0) == '#' && is_num(tmp_row.operand.substr(1)))
			continue;
		if(is_num(tmp_row.operand))
			continue;
		if(tmp_row.operand.at(0) == '@')
			tmp_row.operand = tmp_row.operand.substr(1);
		if(tmp_row.operand.at(0) == '#')
			tmp_row.operand = tmp_row.operand.substr(1);

		string x_delim = ",";
		if(tmp_row.operand.find_first_of(x_delim) != string::npos) 
			tmp_row.operand = tmp_row.operand.substr(0, tmp_row.operand.find_first_of(x_delim));
	
		try {
			tmp_addr = symbol_table.get_addr(tmp_row.operand);
		} catch (...) {
			continue;
		}

		bool continue_mod = true;
		for (unsigned int i = 0; i < equ_mod_fail.size(); i++) {
			if(to_upper(equ_mod_fail[i]) == to_upper(tmp_row.operand))
				continue_mod = false;
		}
		
		if(!continue_mod)
			continue;
		file 	<< "M"
			<< hex_to_string((string_to_hex(tmp_row.addr) + 1),6)
			<< "05"
			<< endl;
	} 

	// Generate End Record
	file << "E" << hex_to_string(program_start, 6);

	// Close File Stream
	file.close();	
}

/*
 * FIRST PASS
 * Assembler First Pass Code
 */
void sicxe_asm::first_pass(file_parser file_tokenized) {
	// Iterator Tracker Variables
	unsigned int pass_addr = 0;	// Program Address (in decimal)
	bool has_start = false; 		// Has program hit start directive
	bool has_end = false;		// Has program hit end directive
	
	
	// Temporary Storage Variables
	int tmp_line;
	string tmp_addr = hex_to_string(pass_addr);
	string tmp_label;
	string tmp_opcode;
	string tmp_operand;
	unsigned int tmp_format_type;
	
	for(int i = 0 ; i < file_tokenized.size() ; i++) {
		tmp_line = i + 1; 
		tmp_addr = to_upper(hex_to_string(pass_addr));		// Current Address
		tmp_label = file_tokenized.get_token(i,0);			// Line Label
		tmp_opcode = to_upper(file_tokenized.get_token(i,1));	// Line OpCode
		tmp_operand = file_tokenized.get_token(i,2);  		// Line Operand
		
		// Code found before or after END / START Directive
		if(!has_start || has_end) {
			bool valid = (tmp_opcode == "START" || tmp_opcode == "END");
			if(!valid && !tmp_opcode.empty())
				throw_error("Assembly code found before START directive or after END directive", tmp_line);
		}
			
		// Start Directive Handler
		if(tmp_opcode == "START") {
			has_start = true;
			program_name = tmp_label;
			tmp_format_type = directive_helper_start(tmp_operand, i);
		} 
		
		// End Directive Handler
		else if(tmp_opcode == "END") {
			has_end = true;
			tmp_format_type = 0;
		} 
		
		// Is OPCODE a Directive?
		else if(directive_check(tmp_opcode)) {
			tmp_format_type = directive_helper(tmp_label, tmp_opcode, tmp_operand, i);
		} 
		
		// Check if OPCODE is empty
		else if(tmp_opcode.empty()) {
			tmp_format_type = 0;
		} 
		
		// Process non-directive instruction
		else {
			tmp_format_type = opcode_index.get_instruction_size(tmp_opcode);
		}
		
		// Address out of bounds check (Only 1MB memory)	
		pass_addr = pass_addr + tmp_format_type;
		if(pass_addr > 0xFFFFF)
			throw_error("Address out of range", tmp_line);
		
		// Add Symbol to symtab	
		if(!tmp_label.empty() && tmp_opcode != "EQU") 
			symbol_table.add_symbol(tmp_label, tmp_addr, i);
				
		// Generate row struct
		first_pass_row tmp_row;
		tmp_row.line 	= tmp_line;
		tmp_row.addr	= tmp_addr;
		tmp_row.label	= tmp_label;
		tmp_row.opcode  = tmp_opcode;
		tmp_row.operand = tmp_operand;
		
		// Push row struct to First Pass Container
		first_pass_vector.push_back(tmp_row);
	}
}

// Check if OPCODE is a Directive
bool sicxe_asm::directive_check(string opcode) {
	const string directives [] = {"START","END","BYTE","WORD","RESB","RESW","BASE","NOBASE","EQU"};
	
	int arr_size = sizeof(directives)/sizeof(directives[0]);
	for(int i = 0; i < arr_size ; i++ ) {
		if(directives[i] == opcode) {
			return true;
		}
	}
	
	return false;
}

unsigned int sicxe_asm::directive_helper(string label, string opcode, string operand, unsigned int line) {
	if(opcode == "BYTE") {
		return directive_helper_byte(operand, line);
	}	
	else if(opcode == "WORD"){
		return 3;
	}
	else if(opcode == "RESB") {
		return directive_helper_resb(operand, line);
	}
	else if(opcode == "RESW"){
		return directive_helper_resw(operand, line);
	}
	else if(opcode == "BASE") {
		return 0;
	}
	else if(opcode == "NOBASE") {
		return 0;
	}
	else if(opcode == "EQU") {
	 	directive_helper_equ(label, operand, line);
		return 0;
	}
	else {
		return 0;
	}
}

/*
 * Directive Helper: START
 * @param string operand	Line Operand
 * @param unsigned int line	Current Line in ASM 
 * @returns unsigned int 	Start Address 
 */
unsigned int sicxe_asm::directive_helper_start(string operand, unsigned int line) {
	program_start = string_to_hex(operand);
	if(operand.at(0) == '$'){
		operand.erase(0,1);
		if(!is_hex(operand))
			throw_error("Invalid operand", line);
			
		return string_to_hex(operand);
	} else if(isdigit(operand.at(0))) {
		return string_to_hex(operand);
	} else {
		throw_error("Invalid operand", line);
	}
}

/*
 * Directive Helper: BYTE
 * @param string operand	Line Operand
 * @param unsigned int line	Current Line in ASM 
 * @returns unsigned int 	Address Change 
 */
unsigned int sicxe_asm::directive_helper_byte(string operand, unsigned int line) {
	string lead = operand.substr(0,2);
	lead = to_upper(lead);
	
	unsigned int disp = 0;
	if(lead == "X'"){
		string bytes = operand.substr(2, operand.size() - 3);
		if(bytes.size() % 2 == 1)
			throw_error("Invalid Operand", line);
		if(!is_hex(bytes))
			throw_error("Invalid Operand", line);
			
		disp = bytes.size() / 2;
	} else if(lead == "C'"){
		string bytes = operand.substr(2, operand.size() - 3);
		disp = bytes.size();
	} else {
		throw_error("Invalid Operand", line);
	}
	
	return disp;
}


unsigned int sicxe_asm::directive_helper_resb(string operand, unsigned int line) {
	unsigned int disp;
	if(operand.at(0) == '$') {
		operand.erase(0,1);
		if(!is_hex(operand))
			throw_error("Invalid Operand", line);
			
		disp = string_to_hex(operand);
	} else if (is_num(operand)) {
		disp = atoi(operand.c_str()); 
	} else {
		throw_error("Invalid Operand", line);
	}
	
	return disp;
}

unsigned int sicxe_asm::directive_helper_resw(string operand, unsigned int line) {
	unsigned int disp;
	if(operand.at(0) == '$') {
		operand.erase(0,1);
		if(!is_hex(operand))
			throw_error("Invalid Operand", line);
			
		disp = string_to_hex(operand);
	} else if (is_num(operand)) {
		disp = atoi(operand.c_str()); 
	} else {
		throw_error("Invalid Operand", line);
	}
	
	disp = disp * 3;
	return disp;
}

void sicxe_asm::directive_helper_equ(string label, string operand, unsigned int line) {
	if(is_num(operand)) {
		string addr = hex_to_string(string_to_int(operand));
		symbol_table.add_symbol(label, addr, line);
	} else if (symbol_table.has_symbol(operand)) {
		symbol_table.add_symbol(label, symbol_table.get_addr(operand),line);
	} else {
		symbol_table.add_symbol(label, operand,line);
	}
	
	return;
}


void sicxe_asm::write_to_file(){	
	// File Stream
	ofstream file;
	stringstream in_stream;
	
	// Remove file extension and append .lis
	size_t last_index = filename.find_last_of("."); 
	string tmp_filename = filename.substr(0, last_index); 
	tmp_filename = tmp_filename + ".lis";
	
	file.open( tmp_filename.c_str() , ios::out );
	if (!file)
		throw_error("Could not create listing file");
	
	// File Formatting
	int BUFFER = 8;
	int COLUMN_BUFFER = 6;

	// File Header
	file << "                            **" << filename << "**                  " << endl;
	file << right << setw(BUFFER) << "Line #" 	<< setw(COLUMN_BUFFER) << " "
	     << right << setw(BUFFER) << "Address" 	<< setw(COLUMN_BUFFER) << " "
	     << left << setw(BUFFER) << "Label"  	<< setw(COLUMN_BUFFER) << " "
	     << left << setw(BUFFER) << "Opcode" 	<< setw(COLUMN_BUFFER) << " "
	     << left << setw(12) << "Operand"  	<< setw(COLUMN_BUFFER) << " "
	     << left << setw(12) << "Machine"  	<< setw(COLUMN_BUFFER) << " "
	     << endl;	     
	file << left << setw(BUFFER) << "========"	<< setw(COLUMN_BUFFER) << " "
	     << left << setw(BUFFER) << "========"	<< setw(COLUMN_BUFFER) << " "
	     << left << setw(BUFFER) << "========" 	<< setw(COLUMN_BUFFER) << " "
	     << left << setw(BUFFER) << "========"	<< setw(COLUMN_BUFFER) << " "
	     << left << setw(12) << "============" 	<< setw(COLUMN_BUFFER) << " "
	     << left << setw(12) << "============" 	<< setw(COLUMN_BUFFER) << " "
	     << endl;
	     
	// Print first pass information     
	for (unsigned int i = 0; i < second_pass_vector.size(); i++) {
		file 	<< right << setw(BUFFER) << int_to_string(second_pass_vector[i].line) << setw(COLUMN_BUFFER)  << " "
	     		<< right << setw(BUFFER) << second_pass_vector[i].addr	<< setw(COLUMN_BUFFER)  << " "
	     		<< left << setw(BUFFER) << second_pass_vector[i].label	<< setw(COLUMN_BUFFER)  << " "
		 	<< left	<< setw(BUFFER)	<< second_pass_vector[i].opcode	<< setw(COLUMN_BUFFER)  << " "
		 	<< left	<< setw(12)	<< second_pass_vector[i].operand << setw(COLUMN_BUFFER)  << " "
			<< left	<< setw(12)	<< second_pass_vector[i].machine << setw(COLUMN_BUFFER)  << " "
			<< endl;
	}
	
	file.close();
}

string sicxe_asm::hex_to_string(unsigned int hex) {
	string tmp_str;
	stringstream ss;
	ss << setw(5) << setfill('0') << std::hex << hex;
	ss >> tmp_str;
	
	if(tmp_str.size() > 5)
		tmp_str = tmp_str.substr((tmp_str.size() - 5));
	return to_upper(tmp_str);
}

string sicxe_asm::hex_to_string(unsigned int hex, unsigned int length) {
	string tmp_str;
	stringstream ss;
	ss << setw(length) << setfill('0') << std::hex << hex;
	ss >> tmp_str;
	
	if(tmp_str.size() > length)
		tmp_str = tmp_str.substr((tmp_str.size() - length));
		
	return to_upper(tmp_str);
}

string sicxe_asm::hex_to_string(int hex, unsigned int length) {
	string tmp_str;
	stringstream ss;
	ss << setw(length) << setfill('0') << std::hex << hex;
	ss >> tmp_str;
	
	if(tmp_str.size() > length)
		tmp_str = tmp_str.substr((tmp_str.size() - length));
		
	return to_upper(tmp_str);
}

string sicxe_asm::hex_to_string_nofill(unsigned int hex) {
	string tmp_str;
	stringstream ss;
	ss << std::hex << hex;
	ss >> tmp_str;
	return to_upper(tmp_str);
}
// Convert string from hex into unsigned int
unsigned int sicxe_asm::string_to_hex(string str) {
	unsigned int tmp_hex;
    	stringstream ss;
    	ss << std::hex << str;
    	ss >> tmp_hex;
	return tmp_hex;
}

unsigned int sicxe_asm::string_to_int(string str) {
	unsigned int tmp_int;
    	stringstream ss;
    	ss << std::dec << str;
    	ss >> tmp_int;
	return tmp_int;
}


string sicxe_asm::int_to_string(unsigned int num) {
	string tmp_int;
    	stringstream ss;
    	ss << std::dec << num;
    	ss >> tmp_int;
	return tmp_int;
}

string sicxe_asm::to_upper(string str){
	transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

bool sicxe_asm::is_num(string str){
	if(str.empty())
		return false;
	if(str.at(0) == '-')
		str.erase(0,1);
		
	for(int i = 0 ; i < str.length() ; i++) {
		if(!isdigit(str.at(i)))
			return false;
	} 
	
	return true;
	
}

bool sicxe_asm::is_hex(string str) {
	if(str.empty())
		return false;

  	for(int i = 0 ; i < str.length() ; i++) {
		if(!isxdigit(str.at(i))) 
			return false;
	}

	return true;
}

unsigned int sicxe_asm::get_label(string label, first_pass_row curr_row) {
	string addr = symbol_table.get_addr(label);
	string tmp_addr = ""; 
	try {
		while(1) {
			tmp_addr = symbol_table.get_addr(addr);
			if(to_upper(addr) == to_upper(tmp_addr))
				break;
			else 
				addr = tmp_addr;
		}
	} catch (...) {}
	
	if(addr.at(0) == '$') 
		addr = addr.erase(0,1);	

	if(!is_hex(addr))
		throw_error("Could not find forward reference", curr_row.line);

	return string_to_hex(addr);
}

/*
 * Error Helpers
 */
void sicxe_asm::throw_error(string msg) {
	msg = "Error occured: " + msg;
	throw symtab_exception(msg);
	exit(0);
}

void sicxe_asm::throw_error(string msg, unsigned int line) {
	string str_line = int_to_string(line);
	msg = "Error occured at line " + str_line +  ": " + msg;
	throw symtab_exception(msg);
	exit(0);
}


// Main Function (Program entry point)
int main(int argc, char *argv[]) {
	if(argc < 2) {
        	cout << "Please provide a file to load!" << endl << "Exiting Program" << endl;
        	exit(1);
	}
	
	new sicxe_asm(argv[1]);
	return 0;
}
