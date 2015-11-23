/**
 * tcpIpCs.cpp
 *
 * http://thisthread.blogspot.com/2011/02/minimal-asio-tcp-server.html
 * http://thisthread.blogspot.com/2011/02/minimal-asio-tcp-client.html
 *
 * http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/tutorial/tutdaytime1.html
 * http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/tutorial/tutdaytime2.html
 */

#include <iostream>
#include <exception>
#include <array>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

class Session : public std::enable_shared_from_this<Session>{
public:
  Session(boost::asio::ip::tcp::socket s):socket_(std::move(s)) {
    std::cerr << " ---### Session ready ###--- " << std::endl;
  }
  
  void start() {
    do_receive();
  }
  
private:
    
  void do_send(std::string msg) {
    auto self(shared_from_this());
    socket_.async_write_some(boost::asio::buffer(msg),
      [this,self](boost::system::error_code error, std::size_t length) {
	if (error) {
	  std::cerr << " ---### Server: Failed sending ###--- " << std::endl;
	  boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	}
	else {
	  std::cerr << " ---### Server: Successful sending ###--- " << std::endl;
	}
      }
    );
  }
  
  void do_receive() {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
      [this,self](boost::system::error_code error, std::size_t length) {
	if (!error) {
	  boost::this_thread::sleep(boost::posix_time::milliseconds(100));

	  //TODO: Successful reading -> send to Samu, then send back Samu's answer
	  std::cerr << " ---### Server: Received packet ###--- " << std::endl;
	  std::cout << "New msg: ";
	  std::cout.write(data_.data(), max_length);
	  std::cout << std::endl;

	  // If Samu's answer were sent, start listening again
	  do_receive();
	}
      }
    );
  }
  
  boost::asio::ip::tcp::socket socket_;
  enum { max_length = 1024 };
  std::array<char, max_length> data_;
};

class Server {
public:
  Server(boost::asio::io_service & io, const short port):socket_(io), acceptor_(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)){
    boost::thread_group tg;
    
    for (int i = 0; i < 5; ++i) {
      tg.create_thread(boost::bind(&boost::asio::io_service::run, &io));
    }
    
    tg.join_all();
    do_accept();
  }
  
  static Server start_server(const short port) {
    
    boost::asio::io_service io_service; 
    boost::asio::io_service::work work(io_service);
    
    Server s(io_service, port);
    
    std::cerr << " ---### Server ready ###--- " << std::endl;
    
    return s;
  }
  
private:
  void do_accept(){
    acceptor_.async_accept(socket_,
      [this](boost::system::error_code error) {
	if (!error) {

	  // Successful connecting with client
	  
	  std::make_shared<Session>(std::move(socket_))->start();

	  // Start listening to clients again
	  do_accept();

	}
      }
    );
  }

  boost::asio::ip::tcp::socket socket_;
  boost::asio::ip::tcp::acceptor acceptor_;
};

class Client : public std::enable_shared_from_this<Client> {
public:
  Client(boost::asio::io_service & io):socket_(io) {  
    
    
    boost::asio::connect(socket_, endpoint);
    
    boost::thread_group tg;
    
    for (int i = 0; i < 5; ++i) {
      tg.create_thread(boost::bind(&boost::asio::io_service::run, &io));
    }
    
    tg.join_all();
    do_receive();
  }
  
  static std::shared_ptr<Client> start_client(const std::string host, const std::string port) {
    boost::asio::io_service io;
    boost::asio::io_service::work work(io);
    boost::asio::ip::tcp::resolver resolver(io);
    endpoint = resolver.resolve(
        boost::asio::ip::tcp::resolver::query(host, port));
    
    std::shared_ptr<Client> c = std::make_shared<Client>(io);

    return c;
  }
  
  void send(std::string msg) {
    std::copy(msg.begin(), msg.end(), buffer_.data());
    do_send(msg.length());
  }
private:
  void do_receive() {
    auto self(shared_from_this());
    std::cout << "alma" << std::endl;
    socket_.async_read_some(boost::asio::buffer(buffer_, max_length),
      [this, self](boost::system::error_code error, std::size_t length) {
	if (!error) {
	  boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	  std::cerr << " ---### Client: Received packet ###--- " << std::endl;
	  std::cout << "New msg: ";
	  std::cout.write(self->buffer_.data(), max_length);
	  std::cout << std::endl;

	  do_receive();
	}
      }
    );
    std::cout << "körte" << std::endl;
  }
  
  void do_send(std::size_t l) {
    auto self(shared_from_this());
    std::cout << "send1" << std::endl;
    socket_.async_write_some(boost::asio::buffer(buffer_, l),
      [this, self](boost::system::error_code error, std::size_t length) {
	if (error) {
	  std::cerr << " ---### Client: Failed sending ###--- " << std::endl;
	  boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	  do_send(length);
	}
	else {
	  std::cerr << " ---### Client: Successful sending ###--- " << std::endl;
	}
      }
    );
    std::cout << "send2" << std::endl;
  }
  
  enum { max_length = 1024 };
  boost::asio::ip::tcp::socket socket_;
  std::array<char,max_length> buffer_;
  static boost::asio::ip::tcp::resolver::iterator endpoint;
};
/*
void asioTcpClient(const char* host)
{
  try
  {
    boost::asio::io_service aios;

    boost::asio::ip::tcp::resolver resolver(aios);
    boost::asio::ip::tcp::resolver::iterator endpoint = resolver.resolve(
        boost::asio::ip::tcp::resolver::query(host, HELLO_PORT_STR));
/*
    boost::asio::ip::tcp::socket socket(aios);
    boost::system::error_code error = boost::asio::error::host_not_found;
    boost::asio::ip::tcp::resolver::iterator end;
    while(error && endpoint != end)
    {
      socket.close();
      socket.connect(*endpoint++, error);
    }
    if(error)
      throw boost::system::system_error(error);
 
    boost::asio::ip::tcp::socket socket(aios);
    // open the connection for the specified endpoint, or throws a system_error
    boost::asio::connect(socket, endpoint);
    
    std::string msg("son was Hezron");
    
    boost::asio::async_write(socket, boost::asio::buffer(msg, msg.length()),
      [](boost::system::error_code ec, std::size_t /*length) {
	if (!ec) {
	  
	}
      });

    for(;;)
    {
      std::array<char, 128> buf;
      boost::system::error_code error;
      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if(error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer
      else if(error)
        throw boost::system::system_error(error);

      std::cout.write(buf.data(), len);
      std::cout << '|';
    }
    std::cout << std::endl;
  }
  catch(std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}
*/
int main(int argc, char* argv[])
{
  if(argc > 1) {
    Server s = Server::start_server(50013);
  } 
  else {
    std::shared_ptr<Client> c = Client::start_client("localhost", "50013");
    c->send("Hello from client");
  }
}
