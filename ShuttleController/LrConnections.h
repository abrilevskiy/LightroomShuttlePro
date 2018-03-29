#pragma once
#include <boost/asio.hpp>
#include <string>
class LrConnections
{
	static boost::asio::ip::tcp::socket sock;
public:
	LrConnections();
	~LrConnections();

	void static Connect();
	void static SendCommand(std::string message);
	void static ShutDown();
};

