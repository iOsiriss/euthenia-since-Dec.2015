#pragma once
#include <atomic>
#include <string>
#include <thread>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>

#include "FlowGen.h"
#include "Session.h"
using namespace std;

class Group
{
public:
	uint32_t time;
	uint32_t sent_flow;
	uint32_t sent_flow_per_sec;
	uint32_t successed_flow_;
	uint32_t successed_PKTIN_;
	uint32_t avg_success_flow_per_sec;
	uint32_t first_packet_time_stamp;
	uint32_t last_packet_time_stamp;
	vector<uint32_t>  buffered_pkt;
	bool running;
	Group(): time(0),sent_flow(0),sent_flow_per_sec(0),successed_flow_(0),successed_PKTIN_(0),avg_success_flow_per_sec(0),first_packet_time_stamp(0),last_packet_time_stamp(0),running(true)
	{
	}
};

class PacketInTester
{
public:
	PacketInTester(FlowGen & flow_gen, Session & session, boost::asio::io_service & ios):session_(session), flow_gen_(flow_gen), ios_(ios), data(2048)
	{
		XID = 0;
	};
	void onMessage(Message);
	void send_hello();
	void send_feature_request(Message msg);
	void test(Message msg);
protected:
	void calculate_buffered_packet(Group &group);
	void group_test(Group &group, uint32_t last_sec, uint32_t packet_count);
	uint32_t get_XID()
	{
		return ++XID;
	}
	void do_test();
	Session & session_;
	FlowGen & flow_gen_;
	boost::asio::io_service & ios_;
	std::atomic_uint_fast32_t XID;
	const int group_interval = 5;
	uint32_t group_index = 0;
	vector<Group> data;
	shared_ptr<std::thread> tester_t;
};

typedef std::shared_ptr<PacketInTester> PktTst_ptr;
