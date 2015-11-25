#include "tcpserver.hpp"


// ----- client_session -----




void Client_session::start_read() {
  std::shared_ptr<Client_session> self(shared_from_this());
  boost::system::error_code err;
  for(;;){
    std::size_t length = socket_.read_some(boost::asio::buffer(data_, max_length), err);
    if(err)
      break;
    std::string gotstr(std::begin(data_), length-2);
    std::string debug = "dbg: ";
    debug += std::to_string(gotstr.length());

    samu.print_console(debug);

    #ifdef DISP_CURSES
      samu.print_console(" ---### Message received from TCP ###--- ");
      samu.print_console(gotstr);
    #endif

    //TODO: maybe id
      std::string response = samu.SamuWorkWithThis(1, gotstr);

      if(response.size() < max_length){
        std::copy(response.begin(), response.end(), data_.begin());
      }else{
        std::copy(response.begin(), response.begin()+max_length, data_.begin());
      }

      //TODO: Successful reading -> send to Samu, then send back Samu's answer
      // If Samu's answer were sent, start listening again

    boost::system::error_code wr_error;
      socket_.write_some(boost::asio::buffer(data_, response.size()), wr_error);

    if (wr_error) {
      break;
    }
    else {
    #ifdef DISP_CURSES
      samu.print_console(" ---### Answer sendback failure ###--- ");
    #endif

      boost::this_thread::sleep(boost::posix_time::milliseconds(100));

    }
  }


  /*socket_.async_read_some(boost::asio::buffer(data_, max_length),
    [this, self](boost::system::error_code error, std::size_t length) {
      if (!error) {
      	boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        //sztringgé alakítja a kapott adatot
        std::string gotstr(std::begin(data_), std::begin(data_)+length);

      #ifdef DISP_CURSES
      	samu.print_console(" ---### Message received from TCP ###--- ");
	samu.print_console(gotstr);
      #endif

	//TODO: maybe id
        std::string response = samu.SamuWorkWithThis(1, gotstr);

        if(response.size() < max_length){
          std::copy(response.begin(), response.end(), data_.begin());
        }else{
          std::copy(response.begin(), response.begin()+max_length, data_.begin());
        }

      	//TODO: Successful reading -> send to Samu, then send back Samu's answer
      	// If Samu's answer were sent, start listening again

	boost::system::error_code wr_error;
        socket_.write_some(boost::asio::buffer(data_, response.size()), wr_error);

	if (!wr_error) {
	  start_read();
	}
	else {
	#ifdef DISP_CURSES
	  samu.print_console(" ---### Answer sendback failure ###--- ");
	#endif
	  //TODO: Write error handling?
	  start_read();
	}

      }
      else {
	samu.print_console(" ---### Read error ###--- ");
	samu.print_console(error.message());

      }
    }
  );*/
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
