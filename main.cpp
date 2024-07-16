#include "INCLUDES.h"

//////////////////////////////////////// NOTE /////////////////////////////////////////////////
// To run one of the examples uncomment the corresponding #define statment as follows:
//         To run the Hello World example: #define HelloWorld
//         To run the Counter example: #define COUNTER
//         To run the TLM example: #define TLM 
//#define TEST_NAME 
///////////////////////////////////////////////////////////////////////////////////////////////

#define HELLOWORLD
//#define COUNTER
//#define TLM


#ifdef HELLOWORLD

#include "HelloWorldExample.cpp"
int sc_main(int, char* [])
{
	cout << "------------------------------ HELLO WORLD MAIN --------------------------------------" << endl;
	Top top("top");
	sc_start(100, SC_NS);
	return 0;
}

#elif defined COUNTER

#include "Simple_Counter_Example.cpp"
int sc_main(int, char* [])
{
	cout << "------------------------------ COUNTER MAIN --------------------------------------" << endl;
	Top top("top");
	sc_start(100, SC_NS);
	return 0;
}

#elif defined TLM

#include "Simple_TLM_Example.cpp"
int sc_main(int, char* [])
{
	cout << "------------------------------ TLM EXAMPLE MAIN --------------------------------------" << endl;
	sc_clock aclk("a", 5, SC_NS);
	Top top("top");
	top.aclk(aclk);
	sc_start(500, SC_NS);
	return 0;
}

#endif
