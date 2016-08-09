/* Andrew Black, Andy Risser, Darren  Barnachea, Kent Vo, Shayan Rastegarpour
    masc0836
    Team Kansas
    prog1
    CS530, Spring 2016
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <cctype>
#include <sstream>
#include "file_parser.h"
#include "file_parse_exception.h"

using namespace std;
//ctor
file_parser::file_parser(string f) {
    filename = f;	// Set filename in class
}

//dtor
file_parser::~file_parser() { }

//reads the file. if unable, throw file_parse_exception
void file_parser::read_file() {
	// Duplicate Call
	if(!token_vector.empty())
		return;
		
    	ifstream infile;

    	char cstr[filename.length()+1];
    	strcpy(cstr, filename.c_str());
    	string line;
    	infile.open(cstr, ios::in);

	// Check if file was found
    	if(!infile.good()) {
        	throw file_parse_exception("in read_file(): file not found");
    	}
	
	// Read to vector loop
    	while(!infile.eof()) {
        	getline(infile,line);
        	buffer_vector.push_back(line);
    	}	
    	infile.close();

    	line_tokenize();
}

/*
 * Purpose: Tokenize comment in each line
 * @params int row		# Row being fetched
 * @params int column 		# Column being fetched
 * @returns token		# Returns token at location
 */
string file_parser::get_token(unsigned int row, unsigned int column) {
	if(token_vector.empty())
		read_file();
		
	if(row >= token_vector.size())
		throw file_parse_exception("at get_token(): row does not exist");

	token_struct tmp_struct = token_vector[row];

	switch ( column ) {
		case 0:
			return tmp_struct.label;
			break;
		case 1:
			return tmp_struct.opcode;
			break;

		case 2:
			return tmp_struct.operand;
			break;
		case 3:
			return tmp_struct.comment;
			break;
		default:
			throw file_parse_exception("in get_token(): please provide a column 0 - 3");
			break;
	}
}

// print source code file to stdout.
void file_parser::print_file() {
	if(token_vector.empty())
		read_file();
		
	int c_width = 14;
	cout.setf(ios::left);
    	for(unsigned int i = 0; i < token_vector.size(); i++) {
        	cout << setw(c_width) << token_vector[i].label   <<
        		setw(c_width) << token_vector[i].opcode  <<
			setw(c_width) << token_vector[i].operand <<
			setw(c_width) << token_vector[i].comment << endl;
    		}
}


// Purpose: Tokenize each row of code
void file_parser::line_tokenize() {
	
	unsigned int line_count = 1;
	for(unsigned int i = 0 ; i < buffer_vector.size() ; i++ ) {
		struct token_struct tmp_struct;
		string *label_addr = &tmp_struct.label;
		string *opcode_addr = &tmp_struct.opcode;
		string *operand_addr = &tmp_struct.operand;
		string *comment_addr = &tmp_struct.comment;
		string line = buffer_vector[i];
		
		line = line_tokenize_comment(line, line_count, comment_addr);	// Tokenize string for comment
		line = line_tokenize_label(line, line_count, label_addr);	// Tokenize string for label
		line = line_tokenize_opcode(line, line_count, opcode_addr);	// Tokenize string for opcode
		line = line_tokenize_operand(line, line_count, operand_addr);	// Tokenize string for operand
		
		string s_delim = " \t";
		unsigned int col_check = line.find_first_not_of(s_delim, 0);
		// Too many columns / Non commented text in 4th column
		if(col_check != - 1) {
			string err = "at line " + int_to_string(line_count) +  ": Invalid Column Format";
                        throw file_parse_exception(err);
		}
			
		line_count++;
		token_vector.push_back(tmp_struct);
	}
}

/*
 * Purpose: Tokenize comment in each line
 * @params string line		# Line being tokenized
 * @params int line_count	# Row being tokenized
 * @params string *comment	# Comment address
 * @returns line		# Returns line with comment removed
 */
string file_parser::line_tokenize_comment(string line, int line_count, string *comment) {
	string c_delim = ".";  	// Comment Delimiter
	unsigned int c_first = line.find_first_of(c_delim, 0); // Check for comment
	if(c_first != string::npos) {
		*comment = line.substr(c_first, (line.size() - c_first)); // Set comment in struct
		line 	= line.substr(0,c_first);
	}
	
	return line; // Return truncated string
}

/*
 * Purpose: Tokenize label in each line
 * @params string line		# Line being tokenized
 * @params int line_count	# Row being tokenized
 * @params string *label	# label address
 * @returns line		# Returns line with comment removed
 */
string file_parser::line_tokenize_label( string line, int line_count, string *label ) {
	// Parse for next generic token
	string *token = new string("");
	string tmp_line = line_search_token(line, token);
	
	// Line has no label
	if(token->length() <= 0)
		return tmp_line; // Return truncated string
	
	// Format Error Checker
	bool err = 0;
	for(int i = 0; i < token->length() ; i++) {
		if(i == 0 && !isalpha((*token)[i])) 
			err = 1;
		if(!isalnum((*token)[i]))
			err = 1;
	}
	
	// If Error found, throw exception 
	if(err) {
		string err = "at line " + int_to_string(line_count) +  ": label not valid";
                throw file_parse_exception(err);
	}
	
	*label = *token; // Set label in struct
	return tmp_line; // Return truncated string
}

/*
 * Purpose: Tokenize comment in each line
 * @params string line		# Line being tokenized
 * @params int line_count	# Row being tokenized
 * @params string *opcode	# Opcode address
 * @returns line		# Returns line with opcode removed
 */
string file_parser::line_tokenize_opcode(string line, int l_count, string *opcode) {
	// Parse for next generic token
	string *token = new string("");
	string tmp_line = line_search_token(line, token);
	
	*opcode = *token; // Set opcode in struct
	return tmp_line;  // Return truncated string
}

/*
 * Purpose: Tokenize comment in each line
 * @params string line		# Line being tokenized
 * @params int line_count	# Row being tokenized
 * @params string *operand	# Operand address
 * @returns line		# Returns line with operand removed
 */
string file_parser::line_tokenize_operand(string line, int l_count, string *operand) {
	// Parse for next generic token
	string *token = new string("");
	string tmp_line = line_search_token(line, token);
	
	// Check for quotes
	string *quote = new string("");
	string tmp_line_quote = line_search_quote(line, token, quote);
	
	// If there was a quote, truncate string appropriately
	string s_delim = " \t";
	int empty_check = tmp_line_quote.find_first_not_of(s_delim, 0);
	if(empty_check == -1)  {
		*operand = *quote;
		 return tmp_line_quote;
	}
		
	*operand = *token; // Set operand in struct
	return tmp_line;   // Return truncated string
}

/*
 * Purpose: Search for quote in each token
 * @params string line		# Line being tokenized
 * @params string *token	# Pointer to token being parsed
 * @params string *quote	# Quote pointer
 * @returns line		# Returns line with quote removed
 */
string file_parser::line_search_quote( string line, string *token, string *quote ) {
	string q_delim = "\'";	// Quotation Delimiters
	int q_first = token->find_first_of(q_delim, 0);
	
	if(q_first != -1) {						// Get quote index in line (originally had index in token)
		int q_last = line.find_last_of(q_delim);		// find closing quote
		*quote = line.substr(0, q_last + 1);
		string ret_line  = line.substr(q_last + 1);
		return ret_line;
	} 
	
	return line;
}

/*
 * Purpose: Search for seperate tokens in each line
 * @params string line		# Line being tokenized
 * @params strng *token		# Pointer to token
 * @returns line		# Returns line with token removed
 */
string file_parser::line_search_token( string line, string *token ) {
	string s_delim = " \t"; // Spacing Delimiters
	
	int token_first = 0;
	int token_last  = line.find_first_of(s_delim, token_first); 
	*token = line.substr(token_first, (token_last - token_first));
	
	int line_first = line.find_first_not_of(s_delim, token_last);
	
	if(line_first != -1)
		return line.substr(line_first);
	else 
		return "";
}

// Purpose: returns number of lines in source code file.
int file_parser::size() {
	if(token_vector.empty())
		read_file();
		
	return token_vector.size();
}

// Purpose: converts integer to string
string file_parser::int_to_string(int n){
        ostringstream outstr; 
        outstr<< n;
        return outstr.str();
}
