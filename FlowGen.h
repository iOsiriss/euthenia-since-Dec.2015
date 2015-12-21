#pragma once
#include <error.h>
#include <arpa/inet.h>
#include <signal.h>
#include <ctime>
#include <climits>
#include <cstring>
#include <string>
#include <iostream>
#include <unistd.h>
#include <pcap.h>
#include <vector>
#include "header_defu.h"
#include "Session.h"

using namespace std;
class FlowGen
{
public:
	FlowGen(char * device_name)
	{
		pd = pcap_open_live(device_name,1514,1,1000,ebuf);
		if(pd == nullptr)
		{
			BOOST_LOG_TRIVIAL(fatal) << "open device failed";
			exit(1);
		}
	};
	FlowGen(std::function<void(Message message)> onMessage):onMessage_(onMessage) {};
	int test(int beg_new_flow, int end_new_flow, int step);
	int a_burst(uint32_t* pkt_sent, uint32_t packet_per_sec, uint32_t last_sec,
	            uint32_t interval = 0, char * dstb = nullptr);

protected:
	const long nsec = 1000000000;
	char ebuf[PCAP_ERRBUF_SIZE];
	bool is_run = true;
	pcap_t * pd = nullptr;
	unsigned int failed = 0;
	unsigned int fector = 1;
	int make_pkt(u_char **pkt_data, u_int *pkt_len);
	int timeval_subtract (timespec *result, timespec * x, timespec *y);
	int timespec_add(timespec *result, timespec x, timespec y);
	int send_packet(uint8_t * pkt_data, uint32_t pkt_len);
	std::function<void(Message message)> onMessage_;
};
