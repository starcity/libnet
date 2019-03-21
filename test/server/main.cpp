#include "net.h"
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <unistd.h>



class Server
{
	public:
		Server(net::io_server *pio_server,const char *ip,uint16_t port):m_tcp(new net::mc_tcp(pio_server,ip,port,net::LISTEN)),m_io_server(pio_server)
		{
			m_tcp->async_accept(new net::mc_tcp(pio_server),std::bind(&Server::handle_accept,this,std::placeholders::_1,std::placeholders::_2));
			m_connects = 0;
		}
		~Server()
		{
		}


		void handle_accept(net::error_code &code,net::mc_tcp *ptcp)
		{
			if(0 == code.ret){

		//	struct sockaddr_in addr = ptcp->get_ori_addr();
		//	std::cout<<"new connect:"<<inet_ntoa(addr.sin_addr)<<":"<<ntohs(addr.sin_port)<<std::endl;
				ptcp->async_read(make_shared<net::buffer>(10),std::bind(&Server::handle_read,this,std::placeholders::_1,std::placeholders::_2));
				m_connects ++;
			}

			m_tcp->async_accept(new net::mc_tcp(m_io_server),std::bind(&Server::handle_accept,this,std::placeholders::_1,std::placeholders::_2));
		}

		void handle_read(net::error_code &code,net::mc_tcp *ptcp)
		{
			if(ptcp->get_status() == net::CLOSED){
				ptcp->async_close(std::bind(&Server::handle_close,this,std::placeholders::_1,std::placeholders::_2));
			}
			else {
			//	std::cout<<ptcp->get_read_buffer()<<std::endl;
				//std::cout<<"total connect:"<<m_connects<<std::endl;
				char *p = new char[4];
				ptcp->async_write(make_shared<net::buffer>(p,4),std::bind(&Server::handle_write,this,std::placeholders::_1,std::placeholders::_2));
			}
		}

		void handle_write(net::error_code &code,net::mc_tcp *ptcp)
		{
			if(code.ret == 0){
	//			char *pstr = ptcp->get_write_buffer();
	//			std::cout<<"send sucessed:"<<pstr<<std::endl;
				ptcp->async_read(make_shared<net::buffer>(),std::bind(&Server::handle_read,this,std::placeholders::_1,std::placeholders::_2));
			}
			else std::cout<<"write failed"<<std::endl;
		}

		void handle_close(net::error_code &code,net::mc_tcp *ptcp)
		{
			struct sockaddr_in addr = ptcp->get_ori_addr();
			std::cout<<"close connect:"<<inet_ntoa(addr.sin_addr)<<":"<<ntohs(addr.sin_port)<<std::endl;

			delete ptcp;
		}


	private:
		net::mc_tcp		*m_tcp;
		net::io_server	*m_io_server;
		int32_t			m_connects;
};


int main()
{
	net::io_server *pio_server = new net::io_server;
	pio_server->init(3);

	Server server(pio_server,"0.0.0.0",5555);

	pio_server->run();
}
