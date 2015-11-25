#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP
#include <memory>
#include <utility>
#define BOOST_ASIO_HAS_MOVE
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

class Samu;

class Client_session : public std::enable_shared_from_this<Client_session>{
public:
  Client_session(boost::asio::ip::tcp::socket socket, Samu &SAMU):socket_(std::move(socket)),samu(SAMU){}
  void start() { start_read(); }
private:
  void start_read();


  boost::asio::ip::tcp::socket socket_;
  enum { max_length = 1024 };
  std::array<char, max_length> data_;
  Samu &samu;
};

class Tcpserver {
public:
  Tcpserver(Samu &SAMU, const short port);
  void stop_server();

private:
  void start_accept();

  Samu & samu;
  boost::thread_group tg;
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::socket socket_;
  boost::asio::ip::tcp::acceptor acceptor_;
};
#include "samu.hpp"
#endif
