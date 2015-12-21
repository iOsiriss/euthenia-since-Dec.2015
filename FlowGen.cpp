#include "FlowGen.h"

int FlowGen::send_packet(uint8_t * pkt_data, uint32_t pkt_len)
{
	Message message = std::make_shared<std::string>((char *)pkt_data, pkt_len);
	onMessage_(message);
	return 0;
}

int FlowGen::a_burst(uint32_t* pkt_sent, uint32_t packet_per_sec, uint32_t last_sec, uint32_t interval, char *dstb)
{
	timespec nsleep = {0,nsec/packet_per_sec};
	timespec sec_beg,sec_end;
	timespec deadline = {0,0};
	timespec result;
	clock_gettime(CLOCK_REALTIME,&sec_beg);
    deadline = sec_beg;
    timespec_add(&deadline, deadline, nsleep);
	while(is_run)
	{
		//determin if terminate this burst
		clock_gettime(CLOCK_REALTIME,&sec_end);
		timeval_subtract(&result, &sec_end , &sec_beg);
		if(result.tv_sec >= last_sec)
		{
			is_run = false;
			break;
		}

		//transmit packets
		for(unsigned int i = 0; i < fector; ++i)
		{
			uint8_t*pkt_data = nullptr;
			uint32_t pkt_len = 0;
			make_pkt(&pkt_data,&pkt_len);
			if(onMessage_ != nullptr)
			{


				send_packet(pkt_data,pkt_len);
			}
			else if(pd != nullptr)
			{
				if(pcap_sendpacket(pd,pkt_data,pkt_len) == -1)
				{
					++failed;
					cerr << pcap_geterr(pd) << endl ;
				}
			}
			else
			{
				BOOST_LOG_TRIVIAL(fatal) << "no sending method specified";

			}
			delete (sniff_ethernet*)pkt_data;
			++*pkt_sent;
		}

		//determine if need sleep
		timespec_add(&deadline, deadline, nsleep);
		timespec now;
		clock_gettime(CLOCK_REALTIME, &now);
			BOOST_LOG_TRIVIAL(debug) << __func__ << ":" <<__LINE__ << " now " <<now.tv_sec << "." << now.tv_nsec << " deadline: " << deadline.tv_sec <<"."<<deadline.tv_nsec;
		if(timeval_subtract(&result,&deadline,&now) != 1)
		{
			//have more time,spleep
			if(nanosleep(&result, NULL) == -1)
			{
				BOOST_LOG_TRIVIAL(debug) << __func__ << ":" <<__LINE__;
				cerr << "nanosleep():" << strerror(errno) << endl;
			}
		}
	}
	return 0;
}



// Subtract the `struct timeval' values X and Y,
// storing the result in RESULT.
// Return 1 if the difference is negative, otherwise 0. */
int FlowGen::timeval_subtract (timespec *result,timespec * x, timespec *y)
{
	/* Perform the carry for the later subtraction by updating y. */
	if (x->tv_nsec < y->tv_nsec)
	{
		int nsec = (y->tv_nsec - x->tv_nsec) / 1000000000 + 1;
		y->tv_nsec -= 1000000000 * nsec;
		y->tv_sec += nsec;
	}
	if (x->tv_nsec - y->tv_nsec > 1000000000)
	{
		int nsec = (x->tv_nsec - y->tv_nsec) / 1000000000;
		y->tv_nsec += 1000000000 * nsec;
		y->tv_sec -= nsec;
	}

	/* Compute the time remaining to wait.
	 *           tv_nsec is certainly positive. */
	result->tv_sec = x->tv_sec - y->tv_sec;
	result->tv_nsec = x->tv_nsec - y->tv_nsec;

	/* Return 1 if result is negative. */
	return x->tv_sec < y->tv_sec;
}

int FlowGen::timespec_add(timespec* result, timespec x, timespec y)
{
	uint8_t inc = 0;
	uint32_t sub = x.tv_nsec + y.tv_nsec;
	if(sub >= nsec)
	{
		sub += nsec;
		inc = 1;
	}

	result->tv_sec = x.tv_sec + y.tv_sec + inc;
	result->tv_nsec = sub;
	return 0;
}


int FlowGen::make_pkt(u_char **pkt_data, u_int *pkt_len)
{
	uint32_t buffer_size = sizeof(sniff_ethernet) + sizeof(sniff_ip) + sizeof(sniff_tcp) + 1400;
	u_char * buffer = new u_char[buffer_size];
	static uint32_t src = UINT_MAX - 1;
	static uint32_t dst = 0;
	sniff_ethernet eth;
	sniff_ip ip;
	sniff_tcp tcp;
	ip.ip_len = sizeof(ip) + sizeof(tcp) + 1400;
	uint32_t *mac_dst = (uint32_t *)(eth.ether_dhost + 2);
	uint32_t *mac_src = (uint32_t *)(eth.ether_shost + 2);
	*mac_src = htonl(src--);
	*mac_dst = htonl(dst++);
	eth.ether_dhost[2] = 0x0c;
	eth.ether_shost[0] = 0x00;
	eth.ether_shost[1] = 0x02;
	eth.ether_shost[2] = 0xb3;
	memcpy(buffer, &eth, sizeof(eth));
	memcpy(buffer + sizeof(eth), &ip, sizeof(ip));
	memcpy(buffer + sizeof(eth) + sizeof(ip), &tcp,sizeof(tcp));
	*pkt_data = buffer;
	*pkt_len = buffer_size;
	return 0;
}

