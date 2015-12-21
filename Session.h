#pragma once
#include <string>
#include <sstream>
#include <memory>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include "openflow.h"
#include "Dispatcher.h"
using boost::asio::ip::tcp;

typedef std::shared_ptr<std::string> Message;

class Session
{
public:
    Session(boost::asio::io_service& io_service, std::string name)
        : io_service_(io_service),socket_(io_service),name_(name)
    {
    }

    tcp::socket& socket()
    {
        return socket_;
    }
    void start();
    void listen(uint16_t port, std::string = "127.0.0.1");
    void connect(uint16_t port, std::string);
    void read();
    void write(Message message);
    ~Session() {};
    void set_dispatcher(Dispatcher & dispatcher);
protected:
    void write_in_io_thread(Message message);
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred, Message message);
    void handle_read_header(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred, ofp_header * header);

protected:
    enum { max_length = 102400 };
    unsigned char data_[max_length];
    boost::asio::io_service & io_service_;
    tcp::socket socket_;
    Dispatcher *dispatcher_;
    std::string name_;
    std::string ip_;
    uint16_t port_;
};
typedef std::shared_ptr<Session> Session_ptr;
