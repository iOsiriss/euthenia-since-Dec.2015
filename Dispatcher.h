#pragma once
#include <arpa/inet.h>
#include <map>
#include <functional>
#include <string>
#include <memory>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include "openflow.h"
#include "utility.h"
class Dispatcher
{
public:
	/* ?*/
	void setCallBackOnMessage(ofp_type type, std::function<void(std::shared_ptr<std::string>)> f)
	{
		funs[type] = f;
		BOOST_LOG_TRIVIAL(debug) << "Dispatcher | " << __func__ << " type " << type << " " << f.target_type().name();
	};

	void setCallBackOnError()
	{

	};

	void setDefaultCallback(std::function<void(std::shared_ptr<std::string> message)> callback)
	{
		defaultCallback = callback;
	};

	
	void onMessage(void * message_ptr )
	{
		ofp_header * header = (ofp_header * )message_ptr;
		size_t length = ntohs(header->length);

		BOOST_LOG_TRIVIAL(trace) << __PRETTY_FUNCTION__ << " | "
		                         << " ofp_type " << (ofp_type)header->type
		                         << " length " << length
		                         << " hex string " << to_hex_string((char*)message_ptr,sizeof(ofp_header));
		std::shared_ptr<std::string> new_message = std::make_shared<std::string>((char *)message_ptr, length);

		/* check if msg type exist?*/
		if(funs.find((ofp_type)header->type) == funs.end())
		{
			BOOST_LOG_TRIVIAL(trace) << "can not find registed call back for message type" << (ofp_type)header->type;
			/* defaultcallback should be set in eproxy.cpp*/
			if(defaultCallback != NULL)
			{
				BOOST_LOG_TRIVIAL(trace) << "Dispatcher | defaultCallback";
				/* send 'new_message' through defaultcallback*/
				defaultCallback(new_message);
			}
			else
			{
				onError();
			}
		}
		else
		{
			BOOST_LOG_TRIVIAL(trace) << __PRETTY_FUNCTION__ << " | " << " call " << funs[(ofp_type)(header->type)].target_type().name();
			funs[(ofp_type)(header->type)](new_message);
		}
	};

	void onError()
	{
		BOOST_LOG_TRIVIAL(warning) << "Dispatcher | onError";
	};

private:
	std::map<ofp_type,std::function<void(std::shared_ptr<std::string> message)>> funs;
	std::function<void(std::shared_ptr<std::string> message)> defaultCallback;
};
