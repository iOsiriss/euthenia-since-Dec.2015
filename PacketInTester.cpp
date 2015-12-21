#include "PacketInTester.h"
#include "openflow.h"
#include "utility.h"
/* generate a hello msg and send out*/
void PacketInTester::send_hello()
{
	char hello[sizeof(ofp_header)];
	ofp_header * hello_ptr = (ofp_header *) hello;
	hello_ptr->type = OFPT_HELLO;
	hello_ptr->version = OFP_VERSION;
	hello_ptr->length = htons(sizeof(ofp_header));
	hello_ptr->xid = htonl(get_XID());
    

	Message hello_str = std::make_shared<std::string>(hello,sizeof(ofp_header));
	session_.write(hello_str);
    BOOST_LOG_TRIVIAL(trace) << __PRETTY_FUNCTION__ << " " << to_hex_string(hello_str->c_str(),sizeof(ofp_header));
}

/* generate a feature request msg from given message and send out*/
void PacketInTester::send_feature_request(Message msg)
{
	/* c_str returns a pointer to an array that contains a null-terminated sequence of characters*/
    ofp_header * header = (ofp_header*)msg->c_str();
	char feature_request[sizeof(ofp_header)];
	ofp_header *feature_request_ptr = (ofp_header *) feature_request;

	feature_request_ptr->version = OFP_VERSION;
	feature_request_ptr->type = OFPT_FEATURES_REQUEST;
	feature_request_ptr->length = htons(sizeof(ofp_header));
	feature_request_ptr->xid = htonl(get_XID());

	Message feature_request_str = std::make_shared<std::string>(feature_request,sizeof(ofp_header));
    BOOST_LOG_TRIVIAL(trace) << __PRETTY_FUNCTION__ << "  ofp_type " << (ofp_type)header->type << " length " << ntohs(header->length);
	session_.write(feature_request_str);
}

/* count pkt-in, update time stamp for each of the message*/
void PacketInTester::onMessage(Message msg)
{
    ofp_header * header = (ofp_header*)msg->c_str();
	timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	if(data[group_index].first_packet_time_stamp == 0)
	{
		data[group_index].first_packet_time_stamp = now.tv_sec;
	}
	data[group_index].last_packet_time_stamp = now.tv_sec;
	data[group_index].successed_PKTIN_++;
    BOOST_LOG_TRIVIAL(trace) << __PRETTY_FUNCTION__ << "  ofp_type " << (ofp_type)header->type << " length " << ntohs(header->length);
}

/* generate a burst of test msg, the time to generate pkt is last_sec(sec)*/
void PacketInTester::group_test(Group & group, uint32_t last_sec, uint32_t packet_count)
{
    BOOST_LOG_TRIVIAL(debug) << __func__;
	flow_gen_.a_burst(&group.sent_flow,  packet_count, last_sec);
    BOOST_LOG_TRIVIAL(debug) << __func__ << "done";
	group.time = last_sec;
	group.sent_flow_per_sec = group.sent_flow/last_sec;
	sleep(group_interval);
	group_index++;
}

/* count number of buffered pkt every 500ms*/
void PacketInTester::calculate_buffered_packet(Group & group)
{
	using namespace boost::asio;
	//every 500ms count thes
	group.buffered_pkt.push_back(group.sent_flow - group.successed_PKTIN_);
	if(group.running)
	{
		shared_ptr<deadline_timer> timer = make_shared<deadline_timer>(ios_);
		timer->expires_from_now(boost::posix_time::millisec(500));
		timer->async_wait(boost::bind(&PacketInTester::calculate_buffered_packet,this,boost::ref(group)));
	}
}

void PacketInTester::do_test()
{

	uint32_t start = 100;
	uint32_t end = 2000;
	uint32_t step = 100;
    BOOST_LOG_TRIVIAL(debug) << __func__;
	for(uint32_t i = start; i <= end; i+=step)
	{
		Group group;
//		calculate_buffered_packet(group);
		group_test(group, 5, i);
		data.push_back(group);
		group.running = false;
	}
}

void PacketInTester::test(Message msg)
{
    ofp_header * header = (ofp_header*)msg->c_str();
    BOOST_LOG_TRIVIAL(debug) <<" PacketInTester | " << __func__<< " ofp_type " <<(ofp_type) header->type << " length " << ntohs(header->length);
	//new thread to do
	tester_t = make_shared<thread>(bind(&PacketInTester::do_test, this));
}
