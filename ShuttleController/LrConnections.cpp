#include "LrConnections.h"

boost::asio::io_service service;
boost::asio::ip::tcp::socket LrConnections::sock = boost::asio::ip::tcp::socket(service);

LrConnections::LrConnections()
{
}


LrConnections::~LrConnections()
{
}


void LrConnections::Connect() {
	printf("Connect is being executed...\r\n");
	//typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

	boost::asio::ip::tcp::endpoint endPointSend(boost::asio::ip::address::from_string("127.0.0.1"), 56789);
	//boost::asio::ip::tcp::endpoint endPointReceive(boost::asio::ip::address::from_string("127.0.0.1"), 56788);
	//boost::asio::ip::tcp::acceptor sendAcceptor(service, sendEndPoint);
	//boost::asio::ip::tcp::acceptor receiveAcceptor(service, sendEndPoint);



	//sock.open(boost::asio::ip::tcp::v4());
	sock.connect(endPointSend);
	printf("Connect is executed.\r\n");


}

void LrConnections::SendCommand(std::string message) {


	std::string stringReply;
	boost::asio::streambuf response;
	size_t sentBytes = boost::asio::write(sock, boost::asio::buffer(message, message.size()));
	std::string data = "\r\n";
	boost::asio::write(sock, boost::asio::buffer(data, data.size()));

	boost::asio::read_until(sock, response, "\r\n");
	std::istream response_stream(&response);
	response_stream >> stringReply;
	size_t availBytes = sock.available();
	printf("Reply: %s. Sent %d, available %d. \r\n", stringReply.c_str(), sentBytes, availBytes);


}

void LrConnections::ShutDown() {
	sock.shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
	sock.close();

}