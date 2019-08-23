#include "asio_server.h"



int main(int argc,char *argv[])
{
	asio_server server("0.0.0.0",5555);
	server.start();
}
