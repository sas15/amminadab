#include "tcpserver.h"

// ----- client_session -----

void client_session::start_read() {
  auto self(shared_from_this());
  socket_.async_read_some(boost::asio::buffer(data_, max_length), 
    [this, self](boost::system::error_code error, std::size_t length) {
      if (!error) {
	boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	
	//TODO: Successful reading -> send to Samu, then send back Samu's answer
	
	// If Samu's answer were sent, start listening again
	start_read();
      }
    }
  );
}

// ----- tcpserver ----- 

boost::thread_group tcpserver::tg;

void tcpserver::start_server(const short port) {
  
  boost::asio::io_service io_service;
  
  tcpserver s(io_service, port);
  
  for (int i = 0; i < 10; ++i) {
    tg.create_thread( [&]{io_service.run();} );
  }
}
  
void tcpserver::stop_server() {
  tcpserver::tg.join_all();
}

  
void tcpserver::start_accept() {
  acceptor_.async_accept(socket_, 
    [this](boost::system::error_code error) {
      if (!error) {
	
	// Successful connecting woth client
	std::make_shared<client_session>(std::move(socket_))->start();
	
	// Start listening to clients again
	start_accept();
	
      }
    }
  );
}
  
