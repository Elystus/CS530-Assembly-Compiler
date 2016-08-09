/*  Andrew Black, Andy Risser, Darren  Barnachea, Kent Vo, Shayan Rastegarpour
    masc0836
    Team Kansas
    prog3
    CS530, Spring 2016
 */
#ifndef SYMTAB_H
#define SYMTAB_H

#include <map>
#include <string>
#include <sstream>
#include "symtab.h"
#include "symtab_exception.h"

using namespace std;

class symtab {
	public:
	
		void add_symbol(string, string, unsigned int);
		bool has_symbol( string );
		string get_addr( string );
	private:
		map<string,string> symtab_map;
	
		string int_to_string( unsigned int );
	
};
#endif
