#include "tcpserver.hpp"


// ----- client_session -----




void Client_session::start_read() {
  std::shared_ptr<Client_session> self(shared_from_this());
  boost::system::error_code err;
  boost::system::error_code wr_error;
  std::string response;
  for(;;){
    
    std::size_t length = socket_.read_some(boost::asio::buffer(data_, max_length), err);
    
    if(err) {
    #ifdef DISP_CURSES
      samu.print_console(" ---### Read failure ###--- ");
      samu.print_console(err.message());
    #endif
      break;
    }
    
    std::string gotstr(std::begin(data_), length);

  #ifdef DISP_CURSES
    samu.print_console(" ---### Message received from TCP ###--- ");
    samu.print_console(gotstr);
  #endif

    //TODO: maybe id
    response = samu.SamuWorkWithThis(1, gotstr);

    socket_.write_some(boost::asio::buffer(response.c_str(), response.size()), wr_error);  

    if (wr_error) {
    #ifdef DISP_CURSES
      samu.print_console(" ---### Answer sendback failure ###--- ");
      samu.print_console(wr_error.message());
    #endif
      //break;
    }
    
  }
}

// ----- tcpserver -----

Tcpserver::Tcpserver(Samu &SAMU, const short port) : samu(SAMU), io_service(),
                                                socket_(io_service),
                                                acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
                                                 {

  boost::asio::io_service::work work(io_service);
  start_accept() ;

  for (int i = 0; i < 10; ++i) {
    tg.create_thread(
              [&](){
                boost::system::error_code err;
                io_service.run(err);
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
	std::make_shared<Client_session>(Client_session(std::move(socket_), samu))->start();
//         Client_session client_session(std::move(socket_), samu);
//         client_session.start();

      	// Start listening to clients again
      	start_accept();

      }
    }
  );
}
