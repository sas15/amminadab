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

class Client_session {
public:
  Client_session(boost::asio::ip::tcp::socket socket):socket_(std::move(socket)){}
  void start() { start_read(); }
private:
  void start_read();

  boost::asio::ip::tcp::socket socket_;
  enum { max_length = 1024 };
  std::array<char, max_length> data_;
};

class Tcpserver {
public:
  Tcpserver(Tcpserver&&c) : socket_(std::move(c.socket_)), acceptor_(std::move(c.acceptor_)) {}
  Tcpserver(const Samu &samu, const short port);
  void stop_server();

private:
  void start_accept();

  boost::thread_group tg;
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::socket socket_;
  boost::asio::ip::tcp::acceptor acceptor_;
};
#include "samu.hpp"
#endif
