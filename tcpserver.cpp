#include "tcpserver.hpp"

// ----- client_session -----

void Client_session::start_read() {
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
    [this](boost::system::error_code error, std::size_t length) {
      if (!error) {
	boost::this_thread::sleep(boost::posix_time::milliseconds(100));

	//TODO: Successful reading -> send to Samu, then send back Samu's answer
#ifdef DISP_CURSES
	Samu::print_console(" ---### Message received from TCP ###--- ");
#endif

	// If Samu's answer were sent, start listening again
	start_read();
      }
    }
  );
}

// ----- tcpserver -----

boost::thread_group Tcpserver::tg;

Tcpserver Tcpserver::start_server(const Samu *samu, const short port) {

  boost::asio::io_service io_service;

  Tcpserver s(io_service, port);

  for (int i = 0; i < 10; ++i) {
    tg.create_thread( [&]{io_service.run();} );
  }
#ifdef DISP_CURSES
  Samu::print_console(" ---### Server started ###--- ");
#endif
  return s;
}

void Tcpserver::stop_server() {
  Tcpserver::tg.join_all();
}


void Tcpserver::start_accept() {
  acceptor_.async_accept(socket_,
    [this](boost::system::error_code error) {
      if (!error) {

      	// Successful connecting woth client

        Client_session client_session(std::move(socket_));
        client_session.start();

      	// Start listening to clients again
      	start_accept();

      }
    }
  );
}
