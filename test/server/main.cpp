#include "net.h"
#include <arpa/inet.h>
#include <iostream>
#include <string.h>



class Server
{
	public:
		Server(net::io_server *pio_server,const char *ip,uint16_t port):m_tcp(new net::mc_tcp(pio_server,ip,port,net::LISTEN)),m_io_server(pio_server)
		{
			m_tcp->async_accept(new net::mc_tcp(pio_server),std::bind(&Server::handle_accept,this,std::placeholders::_1,std::placeholders::_2));
		}
		~Server()
		{
		}


		void handle_accept(int32_t ret,net::mc_tcp *ptcp)
		{
			if(0 == ret){
				struct sockaddr_in addr = ptcp->get_ori_addr();
				std::cout<<"new connect:"<<inet_ntoa(addr.sin_addr)<<":"<<ntohs(addr.sin_port)<<std::endl;

				ptcp->async_read(make_shared<net::buffer>(),std::bind(&Server::handle_read,this,std::placeholders::_1,std::placeholders::_2));
			}

			m_tcp->async_accept(new net::mc_tcp(m_io_server),std::bind(&Server::handle_accept,this,std::placeholders::_1,std::placeholders::_2));
		}

		void handle_read(int32_t ret,net::mc_tcp *ptcp)
		{
			if(ptcp->get_status() == net::CLOSED){
				ptcp->async_close(std::bind(&Server::handle_close,this,std::placeholders::_1,std::placeholders::_2));
			}
			else {
				std::cout<<ptcp->get_read_buffer()<<std::endl;
				char *psend = new char[10];
				sprintf(psend,"i love you");
				ptcp->async_write(make_shared<net::buffer>(psend,strlen(psend)),std::bind(&Server::handle_write,this,std::placeholders::_1,std::placeholders::_2));
			}
		}

		void handle_write(int32_t ret,net::mc_tcp *ptcp)
		{
			if(ret == 0){
				char *pstr = ptcp->get_write_buffer();
				std::cout<<"send sucessed:"<<pstr<<std::endl;
				ptcp->async_read(make_shared<net::buffer>(),std::bind(&Server::handle_read,this,std::placeholders::_1,std::placeholders::_2));
			}
		}

		void handle_close(int ret,net::mc_tcp *ptcp)
		{
			struct sockaddr_in addr = ptcp->get_ori_addr();
			std::cout<<"close connect:"<<inet_ntoa(addr.sin_addr)<<":"<<ntohs(addr.sin_port)<<std::endl;

			delete ptcp;
		}


	private:
		net::mc_tcp		*m_tcp;
		net::io_server	*m_io_server;
};


int main()
{
	net::io_server *pio_server = new net::io_server;
	pio_server->init(3);

	Server server(pio_server,"0.0.0.0",5555);

	pio_server->run();
}
