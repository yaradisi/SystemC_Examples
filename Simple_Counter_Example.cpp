#include "INCLUDES.h"

//////////////////////////////////////// COUNTER EXAMPLE /////////////////////////////////////////////////
// This code serves as a simple example to get familiar with system c
// It implements a simple counter that increments count output each posedge of the clk
////////////////////////////////////////////////////////////////////////////////////////////////////////////

SC_MODULE(Simple_Counter_Module) {

	sc_in<bool> aclk;
	sc_in<bool> resetn;
	sc_out<int> counter;

	void counter_thread() {
		counter = 0;
		while (true) {
			wait();
			if (!resetn) {
				counter = 0;
				cout << "-------------------> Simple_Counter_Module: RESET! Counter: " << counter << endl;
			}
			else {
				counter = counter + 1;
				cout << "-------------------> Simple_Counter_Module: Counter++ = " << counter << endl;
			}

		}
	}
	
	// constructor
	SC_CTOR(Simple_Counter_Module) {

		SC_THREAD(counter_thread);
		sensitive << aclk.pos();

	}
};

SC_MODULE(Top) {

	sc_clock aclk;
	sc_signal<bool> resetn;
	sc_signal<int> count;
	void print_count() {
		while (true) {
			wait(aclk.posedge_event());
			cout << "-------------------> Top_Module: count: " <<  count << endl;
		}
	}

	Simple_Counter_Module counter;

	// constructor
	SC_CTOR(Top) : aclk("aclk", 5, SC_NS), counter("counter") {
		counter.aclk(aclk);
		counter.resetn(resetn);
		counter.counter(count);

		SC_THREAD(print_count);
		sensitive << aclk; // Sensitivity to the rising edge of the clock

		// initialize reset: 
		resetn = 1;
	}
};


