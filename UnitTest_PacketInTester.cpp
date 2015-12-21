#include <thread>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include "Dispatcher.h"
#include "FlowGen.h"
#include "PacketInTester.h"
#include "openflow.h"

void boost_log_init(int severity = 2)
{
	namespace logging = boost::log;
	namespace keywords = boost::log::keywords;
	namespace sinks = boost::log::sinks;
	logging::core::get()->set_filter
	(
	    logging::trivial::severity >= severity
	);
}

int main(int argc, char * argv[])
{
	if( argc < 2)
	{
		printf("tester device_name\n");
		return 1;
	}
    if( argc > 3){
       int severity =  atoi(argv[2]);
      boost_log_init(severity);
    }

	boost::asio::io_service io_service;
    //Flow generator will send packet on given device
	FlowGen generator(argv[1]);
    //maintain a session between openflow switch and tester for sending
    //control message
	Session s_session(io_service, "switch_session");
	Dispatcher dispatcher_from_switch;

	// send packet through generator, send control message from s_session
	PacketInTester tester(generator, s_session, io_service);

    //when get OFPT_HELLO response send feature request
	dispatcher_from_switch.setCallBackOnMessage(OFPT_HELLO,
	        std::bind(
	            &PacketInTester::send_feature_request,
	            &tester,
	            std::placeholders::_1
	        ));
    //when get OFPT_FEATURES_REPLY we can start to test
	dispatcher_from_switch.setCallBackOnMessage(OFPT_FEATURES_REPLY,
	        std::bind(
	            &PacketInTester::test,
	            &tester,
	            std::placeholders::_1
	        ));
	//when get packet :  s_session -> dipatcher -> tester.onMessage
	dispatcher_from_switch.setCallBackOnMessage(OFPT_PACKET_IN,
	        std::bind(
	            &PacketInTester::onMessage,
	            &tester,
	            std::placeholders::_1
	        ));
    //all coming packets will send to dispatcher for dispacting 
	s_session.set_dispatcher(dispatcher_from_switch);

    //listing on 9000 for incoming OFP siwtch
    BOOST_LOG_TRIVIAL(info) << "waiting Openflow switch connection...";
    s_session.listen(9000);
    BOOST_LOG_TRIVIAL(info) << "switch connected";
    //start test
    tester.send_hello();
    s_session.start();
    BOOST_LOG_TRIVIAL(info) << "start io service";
	io_service.run();

	return 0;
}
