/* Andrew Black, Andy Risser, Darren  Barnachea, Kent Vo, Shayan Rastegarpour
    masc0836
    Team Kansas
    prog2
    CS530, Spring 2016
 */

#ifndef OPCODE_ERROR_EXCEPTION_H
#define OPCODE_ERROR_EXCEPTION_H

#include <string>

using namespace std;

class opcode_error_exception{

public:
	opcode_error_exception(string s){
		message=s;
		}

	opcode_error_exception(){
		message="An error has occurred";
		}

	string getMessage(){
		return message;
		}

private:
	string message;
};

#endif
