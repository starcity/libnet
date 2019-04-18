#include "net.h"
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <unistd.h>



class Server
{
	public:
		Server(net::io_service *pio_service,const char *ip,uint16_t port):m_tcp(new net::mc_tcp(pio_service,net::end_point(ip,port),net::LISTEN)),m_io_service(pio_service)
		{
			net::mc_tcp *tcp_cli = new net::mc_tcp(pio_service);
			m_tcp->async_accept(tcp_cli,std::bind(&Server::handle_accept,this,std::placeholders::_1,std::placeholders::_2));
		}
		~Server()
		{
		}


		void handle_accept(net::error_code &code,net::mc_tcp *ptcp)
		{
			if(!code){
				ptcp->async_read(make_shared<net::buffer>(m_read,1024),std::bind(&Server::on_read_client,this,std::placeholders::_1,std::placeholders::_2));

				net::mc_tcp *tcp_cli = new net::mc_tcp(m_io_service);
			m_tcp->async_accept(tcp_cli,std::bind(&Server::handle_accept,this,std::placeholders::_1,std::placeholders::_2));
			}
			else std::cout<<"accept failed"<<std::endl;
		}

		void on_read_client(net::error_code &code,net::mc_tcp *ptcp)
		{
			if(!code){
				ptcp->async_write(make_shared<net::buffer>(ptcp->get_read_buffer(),ptcp->get_read_buffer_len()),std::bind(&Server::on_write_client,this,std::placeholders::_1,std::placeholders::_2));
			}
			else {
				ptcp->async_close(std::bind(&Server::handle_close,this,std::placeholders::_1,std::placeholders::_2));
			}
		}

		void on_write_client(net::error_code &code,net::mc_tcp *ptcp)
		{
			if(!code){
				ptcp->async_read(make_shared<net::buffer>(m_read,1024),std::bind(&Server::on_read_client,this,std::placeholders::_1,std::placeholders::_2));
			}
			else {
				ptcp->async_close(std::bind(&Server::handle_close,this,std::placeholders::_1,std::placeholders::_2));
				std::cout<<"on_write_client failed"<<std::endl;
			}
		}

		void handle_close(net::error_code &code,net::mc_tcp *ptcp)
		{
			net::end_point ep = ptcp->get_ori_addr();
			std::cout<<"close connect:"<<ep.ip_to_string()<<":"<<ep.port()<<std::endl;

			delete ptcp;
		}


	private:
		net::mc_tcp		*m_tcp;
		net::io_service	*m_io_service;
		uint8_t		    m_buf[1024 * 1024];
		uint8_t			m_read[1024];
};


int main()
{
	net::io_service *pio_service = new net::io_service;
	pio_service->init();

	Server server(pio_service,"0.0.0.0",5555);

	pio_service->run();
}
