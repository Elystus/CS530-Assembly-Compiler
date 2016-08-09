/* Andrew Black, Andy Risser, Darren  Barnachea, Kent Vo, Shayan Rastegarpour
    masc0836
    Team Kansas
    prog1
    CS530, Spring 2016
 */
#ifndef FILE_PARSER_H
#define FILE_PARSER_H

#include <string>
#include <vector>
using namespace std;

struct token_struct;

class file_parser {
public:

    file_parser(string);

    ~file_parser();


    void read_file();


    string get_token(unsigned int, unsigned int);


    void print_file();


    int size();

private:
    // Private Structure
    struct token_struct {
    public:
        // Constructor
        token_struct () {
            label = opcode = operand = comment = ""; // Initialize all variables
        };
	
	string label;	// Column 0
	string opcode;	// Column 1
	string operand; // Column 2
	string comment; // Column 3
    };

    // Private Variables
    string filename;			// file name/source
    vector<string> buffer_vector;	// Step 1: Stored Lines
    vector<token_struct> token_vector;	// Step 2: Tokenized Lines

    // Public Functions
    void line_tokenize();
    string line_tokenize_comment(string, int, string *comment);
    string line_tokenize_label(string, int, string *label);
    string line_tokenize_opcode(string, int, string *opcode);
    string line_tokenize_operand(string, int, string *operand);
    string line_search_quote(string, string *token, string *quote);
    string line_search_token(string, string *token);
    string int_to_string(int);
};

#endif
