/*  Andrew Black, Andy Risser, Darren  Barnachea, Kent Vo, Shayan Rastegarpour
    masc0836
    Team Kansas
    prog3
    CS530, Spring 2016
 */
#include <map>
#include <sstream>
#include "symtab.h"
#include "symtab_exception.h"

void symtab::add_symbol(string label, string addr, unsigned int line) {
	map<string,string>::iterator symtab_iter = symtab_map.find(label);
	if(symtab_iter != symtab_map.end()) 
		throw symtab_exception("Line " + int_to_string(line) + " - Duplicate symbol found: " + label);
	
	symtab_map.insert(pair<string, string>(label, addr));		
}

bool symtab::has_symbol(string str) {
	map<string,string>::iterator symtab_iter = symtab_map.find(str);
	
	return (symtab_iter != symtab_map.end());
}

string symtab::get_addr(string str) {
	map<string,string>::iterator symtab_iter = symtab_map.find(str);
	if(symtab_iter == symtab_map.end()) 
		throw symtab_exception("Symbol not found in symbol table: " + str);
	return symtab_map.find(str)->second;
}

// Convert string from hex into unsigned int
string symtab::int_to_string(unsigned int line) {
	string str;
	stringstream ss;
	ss << line;
	ss >> str;
	return str;
}
