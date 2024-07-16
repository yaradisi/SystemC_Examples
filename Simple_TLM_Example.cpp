#include "INCLUDES.h"

//////////////////////////////////////////////////////// TLM EXAMPLE ///////////////////////////////////////////////////////// 
// This example serves as an introduction to tlm generic payload and sc_events 
// implementing a master module that contains an initiator socket which a user defined master packet is sent through
// and a slave module which contains a target socket which the master packet is read from
// and a top module where the initiator and target sockets are binded in 
// The example also shows the implementation of System C events (sc_event), how to trigger them (using .notify()), declare
// them in sensitivity list and wait for them
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct master_pkt {
	int  master_id;
	sc_uint<32> data;
};

template <int MASTER_ID> // Parameter
SC_MODULE(Master) {

	sc_in<bool> aclk;
	sc_in<bool> resetn;
	sc_in<bool> write_en;

	master_pkt masterpkt;
	tlm_utils::simple_initiator_socket<Master> masterpkt_socket; //initiator = out

	sc_event trigger_sending_masterpkt_event;

	//----------------------------------------------------------------------------------------------------------
	// This thread function checks for reset, if reset it resets the master packet, 
	// else if write_en is high it fills the master packet and triggers the writing to socket thread
	// using sc_event_name.notify()
	//----------------------------------------------------------------------------------------------------------
	void write() {
		sc_uint<32> i = 0;
		masterpkt.master_id = MASTER_ID;
		masterpkt.data = 0;
		while (true) {
			wait();
			if (!resetn) {
				masterpkt.master_id = MASTER_ID;
				masterpkt.data = 0;
				cout << "-------------------> Master_Module: RESET! " << endl;
			}
			else if (write_en) {
				masterpkt.master_id = MASTER_ID;
				masterpkt.data = i;
				cout << "-------------------> Master_Module: write thread -> packet UPDATED: Master_ID: " << masterpkt.master_id << ", Data: " << masterpkt.data << endl;
				trigger_sending_masterpkt_event.notify(); //trigger event
				i = i + 1;
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------
	// This thread transports transaction through tlm socket using b_transport function
	//----------------------------------------------------------------------------------------------------------
	void send_masterpkt() {
		tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
		sc_time delay = sc_time(10, SC_NS);

		// Set the payload fields
		trans->set_address(0);
		trans->set_data_length(sizeof(master_pkt));
		trans->set_data_ptr(reinterpret_cast<unsigned char*>(&masterpkt));
		trans->set_command(tlm::TLM_WRITE_COMMAND);

		// Send the payload through the TLM port
		masterpkt_socket->b_transport(*trans, delay);

		cout << "-------------------> Master_Module: send_masterpkt method -> packet TRANSPORTED: Master_ID: " << masterpkt.master_id << ", Data: " << masterpkt.data << endl;
		if (trans->is_response_error())
			SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
	}

	SC_CTOR(Master) : masterpkt_socket("masterpkt_socket") {

		SC_THREAD(write);
		sensitive << aclk.pos();

		SC_METHOD(send_masterpkt);
		sensitive << trigger_sending_masterpkt_event;
		dont_initialize();
	}
};

SC_MODULE(Slave) {

	master_pkt masterpkt;
	tlm_utils::simple_target_socket<Slave> masterpkt_socket; //target = in

	sc_event trigger_printing_received_packet_event;
	sc_event indicate_printing_done_event;
	sc_event wrap_up_printing_event;

	//-------------------------------------------------------------------------------------------
	// This thread fucntion is triggered by an event that is notified in the read packet method
	//  whenever a packet is received throught the socket, it prints the packet content
	// then triggers another threads and waits for another event that is notified in the 
	// triggered event
	//-------------------------------------------------------------------------------------------
	void print_packet_received() {
		while (true) {
			wait(trigger_printing_received_packet_event);
			cout << "---> SLAVE: Master packet contents: Master_ID: " << masterpkt.master_id << ", Data: " << masterpkt.data << endl;
			indicate_printing_done_event.notify();
			
			wait(wrap_up_printing_event);
			cout << "---> SLAVE: wrapping up print_packet_received thread" << endl;
		}
	}
	void printing_done() {
		while (true) {
			wait();
			cout << "---> SLAVE: printing done!" << endl;
			wrap_up_printing_event.notify();
		}
	}
	//---------------------------------------
	// RECEIVE MASTER PACKET THROUGH SOCKET
	//---------------------------------------
	virtual void readmasterpkt(tlm::tlm_generic_payload & trans, sc_time & delay)
	{
		tlm::tlm_command cmd = trans.get_command();
		sc_dt::uint64    adr = trans.get_address();
		unsigned char* ptr = trans.get_data_ptr();
		masterpkt = *reinterpret_cast<master_pkt*>(trans.get_data_ptr());
		unsigned int     len = trans.get_data_length();
		unsigned char* byt = trans.get_byte_enable_ptr();
		unsigned int     wid = trans.get_streaming_width();

		// Set response status to indicate successful completion
		trans.set_response_status(tlm::TLM_OK_RESPONSE);
		trigger_printing_received_packet_event.notify();
	}


	SC_CTOR(Slave) : masterpkt_socket("masterpkt_socket") {

		//SC_THREAD(FUCNTION_NAME)
		SC_THREAD(print_packet_received);
		sensitive << trigger_printing_received_packet_event << wrap_up_printing_event;


		SC_THREAD(printing_done);
		sensitive << indicate_printing_done_event;


		masterpkt_socket.register_b_transport(this, &Slave::readmasterpkt);
	}
};



SC_MODULE(Top) {

	// sc_signals declared to connect to the master module inputs and outputs 
	sc_in<bool> aclk;
	sc_signal<bool> resetn;
	sc_signal<bool> write_en;

	Master <3> master;
	Slave slave;

	// Thread funtion to toggle master write_en
	void trigger_write() {
		write_en = 0;
		while (true) {
			wait();
			write_en = 1;
			cout << "-------------------> Top_Module: WRITE ENABLED" << endl;
			wait();
			write_en = 0;
			cout << "-------------------> Top_Module: WRITE DISABLED" << endl;
			wait();
			wait();
		}
	}

	// constructor
	SC_CTOR(Top) :master("master"), slave("slave") {

		// connect the master inputs and outputs to the sc_signals declared above 
		master.aclk(aclk);
		master.resetn(resetn);
		master.write_en(write_en);

		master.masterpkt_socket.bind(slave.masterpkt_socket);

		SC_THREAD(trigger_write);
		sensitive << aclk.neg(); // Sensitivity to the rising edge of the clock

		// initialize reset: 
		resetn = 1;
	}
};


