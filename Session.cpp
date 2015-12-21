#include <arpa/inet.h>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include "Session.h"
#include "Dispatcher.h"
#include "openflow.h"
void Session::listen(uint16_t port, std::string ip)
{

	using boost::asio::ip::tcp;
	port_ = port;
	ip_ = ip;
	tcp::acceptor acpt(io_service_,tcp::endpoint(tcp::v4(),port));
	acpt.accept(socket_);
}

void Session::connect(uint16_t port, std::string ip)
{
	using namespace boost::asio::ip;
	port_ = port;
	ip_ = ip;
	socket_.connect(tcp::endpoint(address::from_string(ip), port));
}

void Session::start()
{
	BOOST_LOG_TRIVIAL(trace) << name_ <<" " << __PRETTY_FUNCTION__;
	read();
}

void Session::set_dispatcher(Dispatcher& dispatcher)
{
	dispatcher_ = &dispatcher;
}

void Session::read()
{
	BOOST_LOG_TRIVIAL(trace) << name_ <<" " << __PRETTY_FUNCTION__;
	boost::asio::async_read(socket_,
	                        boost::asio::buffer(data_,sizeof(ofp_header)),
	                        boost::asio::transfer_exactly(sizeof(ofp_header)),
	                        boost::bind(&Session::handle_read_header,
	                                    this,
	                                    boost::asio::placeholders::error,
	                                    boost::asio::placeholders::bytes_transferred)
	                       );
}

void Session::handle_read_header(const boost::system::error_code& error,
                                 size_t bytes_transferred)
{
	ofp_header* header = (ofp_header *)data_;
	BOOST_LOG_TRIVIAL(debug) << name_ << " " << __PRETTY_FUNCTION__ << " | "
	                         << " type " << ofp_type(header->type)
	                         << " length " << ntohs(header->length)
	                         << " xid " << ntohl(header->xid)
	                         << " hex string " << to_hex_string((char*)data_,sizeof(ofp_header));

	size_t length = ntohs(header->length) - sizeof(ofp_header);
	if(length == 0)
	{
		BOOST_LOG_TRIVIAL(trace) << name_ <<" "<<__PRETTY_FUNCTION__ <<" | OFP Message Body length 0";
		dispatcher_->onMessage(data_);
		read();

	}
	else
	{
		BOOST_LOG_TRIVIAL(trace) << name_ << " | " << __PRETTY_FUNCTION__ << " read body";
		boost::asio::async_read(socket_,
		                        boost::asio::buffer(data_ + sizeof(ofp_header), length),
		                        boost::asio::transfer_exactly(length),
		                        boost::bind(&Session::handle_read,this,
		                                    boost::asio::placeholders::error,
		                                    boost::asio::placeholders::bytes_transferred,
		                                    header));
	}
}

void Session::handle_read(const boost::system::error_code& error,
                          size_t bytes_transferred,
                          ofp_header * header)
{
	size_t length = ntohs(header->length);
	BOOST_LOG_TRIVIAL(debug) << name_ << " " << __PRETTY_FUNCTION__ << " | "<< " bytes_transferred " << bytes_transferred<< " hex sring " << to_hex_string((char *)data_ + 8, bytes_transferred);
	if (!error)
	{
		dispatcher_->onMessage(data_);
		read();
	}
	else
	{
		BOOST_LOG_TRIVIAL(fatal) << name_ << "  "<<__PRETTY_FUNCTION__<< " | " << error.message();
	}
}

void Session::write(Message message)
{
	BOOST_LOG_TRIVIAL(trace) << name_ << " | " << __func__;
	io_service_.post(boost::bind(&Session::write_in_io_thread,this,message));
}

void Session::write_in_io_thread(Message message)
{
	ofp_header * header = (ofp_header*)message->c_str();
	BOOST_LOG_TRIVIAL(debug) << name_ << " | write_in_io_thread : "
	                         << " type " << ofp_type(header->type)
	                         << " length " << ntohs(header->length)
	                         << " xid " << ntohl(header->xid)
	                         << " hex string " << to_hex_string((char*)header,sizeof(ofp_header));

	boost::asio::async_write(this->socket(),
	                         boost::asio::buffer(message->c_str(), message->length()),
	                         boost::bind(&Session::handle_write,
	                                     this,
	                                     boost::asio::placeholders::error,
	                                     boost::asio::placeholders::bytes_transferred,
	                                     message
	                                    )
	                        );
}

void Session::handle_write(const boost::system::error_code& error,
                           size_t bytes_transferred,
                           Message message)
{
	if (!error)
	{
		BOOST_LOG_TRIVIAL(debug) << name_ << " | handle_write : " << to_hex_string(message->c_str(), message->length());
	}
	else
	{
		BOOST_LOG_TRIVIAL(fatal) << name_ << " | handle_write error " << error.message() << std::endl;
	}
}
