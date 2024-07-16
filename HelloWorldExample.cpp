#include "INCLUDES.h"

//////////////////////////////////////// HELLOWORLD EXAMPLE /////////////////////////////////////////////////
// This example serves as an introduction to System C modules, constructors and
//  explores the difference between threads and methods in System C and how to implement them
// Using the following System C macros: SC_MODULE, SC_CTOR, SC_CLOCK, SC_THREAD and SC_METHOD
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

SC_MODULE(Hello_World_Module) {

	sc_in<bool> aclk;

	void initilization_thread() {
		
		cout << "---------------> Hello World Example initialization" << endl;
	}
	
	// This thread will only be executed once, triggered by the first positive edge of the clk
	void hello_world_thread() {
		wait();
		cout << "---------------> THREAD: Hello World!" << endl;
	}

	//This infinite loop thread will be executed at every posedge of the clk
	void hello_world_infinite_thread() {
		int i = 0;
		while (true) {
			wait();
			cout << "---------------> INFINITE THREAD: Hello World! #" << i << endl;
			i++;
		}
	}

	// This method will be executed each time the posedge of the clk event occurs, no while loop needed
    int j = 0;
	void hello_world_method() {
		cout << "---------------> METHOD: Hello World! #" << j << endl;
		j++;
	}

	// constructor
	SC_CTOR(Hello_World_Module) {

		// without dont_initialize(); method this thread will be executed immediately at simulation start
		SC_THREAD(initilization_thread);

		SC_THREAD(hello_world_thread);
		sensitive << aclk.pos();
		dont_initialize(); // this method ensures the thread isnt executed immediately at simulation start

		SC_THREAD(hello_world_infinite_thread);
		sensitive << aclk.pos();
		dont_initialize();

		SC_METHOD(hello_world_method);
		sensitive << aclk.pos();
		dont_initialize(); // this method ensures the thread isnt executed immediately at simulation start

	}

};
// NOTE: mixing sc_threads with sc_methods in the same code is not the best practice since each has a different scheduling behavior

SC_MODULE(Top) {

	sc_clock aclk;

	// instantiation
	Hello_World_Module hw;

	// constructor
	SC_CTOR(Top) : aclk("aclk", 5, SC_NS), hw("hw") {
		hw.aclk(aclk);
	}
};



