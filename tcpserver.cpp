#include "tcpserver.hpp"


// ----- client_session -----




void Client_session::start_read() {
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
    [this](boost::system::error_code error, std::size_t length) {
      if (!error) {
      	boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        std::string gotstr(std::begin(data_), std::begin(data_)+length);
        //TODO: maybe id
        std::string response = samu.SamuWorkWithThis(1, gotstr);
        std::array<char, max_length> resp_data;

        if(response.size() < max_length){
          std::copy(response.begin(), response.end(), resp_data.begin());
        }else{
          std::copy(response.begin(), response.begin()+max_length, resp_data.begin());
        }

      	//TODO: Successful reading -> send to Samu, then send back Samu's answer
      #ifdef DISP_CURSES
      	samu.print_console(" ---### Message received from TCP ###--- ");
      #endif
      	// If Samu's answer were sent, start listening again

        boost::asio::async_write(socket_,
          boost::asio::buffer(resp_data, (response.size() < max_length ? response.size() : max_length)),
            [this](boost::system::error_code error, std::size_t length) {
              start_read();
            }
          );
      	//start_read();
      }
    }
  );
}

// ----- tcpserver -----

Tcpserver::Tcpserver(Samu &SAMU, const short port) : samu(SAMU), io_service(),
                                                socket_(io_service),
                                                acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
                                                 {
  start_accept() ;

  boost::asio::io_service::work work(io_service);

  for (int i = 0; i < 10; ++i) {
    tg.create_thread(
              [&](){
                io_service.run();
              }
        );
  }
  #ifdef DISP_CURSES
  samu.print_console(" ---### Server started ###--- ");
  #endif
}

void Tcpserver::stop_server() {
  tg.join_all();
}

void Tcpserver::start_accept() {
  acceptor_.async_accept(socket_,
    [this](boost::system::error_code error) {
      if (!error) {

      	// Successful connecting woth client

        Client_session client_session(std::move(socket_), samu);
        client_session.start();

      	// Start listening to clients again
      	start_accept();

      }
    }
  );
}
