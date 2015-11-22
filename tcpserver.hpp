#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "samu.hpp"

class Client_session: public std::enable_shared_from_this<Client_session> {
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
  Tcpserver(boost::asio::io_service & io_service, const short port):socket_(io_service),acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
    start_accept() ;
  }
  static Tcpserver start_server(Samu & samu, const short port);
  static void stop_server();

private:
  void start_accept();
  
  boost::asio::ip::tcp::socket socket_;
  boost::asio::ip::tcp::acceptor acceptor_;
  static boost::thread_group tg;
};
