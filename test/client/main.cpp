#include "net.h"
#include <arpa/inet.h>
#include <iostream>
#include <string.h>

int32_t flag = 0;

class Client
{
	public:
		Client(net::io_server *pio_server,const char *ip,uint16_t port)
		{
			net::mc_tcp		*m_tcp = new net::mc_tcp(pio_server,ip,port);
			m_tcp->async_connect(std::bind(&Client::handle_connect,this,std::placeholders::_1,std::placeholders::_2));
		}
		~Client()
		{
		}


		void handle_connect(int32_t ret,net::mc_tcp *ptcp)
		{
			if(0 == ret){
				struct sockaddr_in addr = ptcp->get_ori_addr();
				std::cout<<"new connect:"<<inet_ntoa(addr.sin_addr)<<":"<<ntohs(addr.sin_port)<<std::endl;
				char *psend = new char[50];
				sprintf(psend,"i love you");
				
				ptcp->async_write(make_shared<net::buffer>(psend,strlen(psend)),std::bind(&Client::handle_write,this,std::placeholders::_1,std::placeholders::_2));
			}
		}

		void handle_read(int32_t ret,net::mc_tcp *ptcp)
		{
			if(ptcp->get_status() == net::CLOSED){
				ptcp->async_close(std::bind(&Client::handle_close,this,std::placeholders::_1,std::placeholders::_2));
			}
			else {
				flag ++;
				if(flag > 1000){
					std::cout<<flag<<std::endl;
					ptcp->async_close(std::bind(&Client::handle_close,this,std::placeholders::_1,std::placeholders::_2));
					return;
				}
				else 
					std::cout<<flag<<std::endl;
				std::cout<<ptcp->get_read_buffer()<<std::endl;

				char *psend = new char[10];
				sprintf(psend,"i love you");
				ptcp->async_write(make_shared<net::buffer>(psend,strlen(psend)),std::bind(&Client::handle_write,this,std::placeholders::_1,std::placeholders::_2));
			}
		}

		void handle_write(int32_t ret,net::mc_tcp *ptcp)
		{
			if(ret == 0){
				char *pstr = ptcp->get_write_buffer();
				std::cout<<"send sucessed:"<<pstr<<std::endl;
				ptcp->async_read(make_shared<net::buffer>(),std::bind(&Client::handle_read,this,std::placeholders::_1,std::placeholders::_2));
			}
		}

		void handle_close(int ret,net::mc_tcp *ptcp)
		{
			struct sockaddr_in addr = ptcp->get_ori_addr();
			std::cout<<"close connect:"<<inet_ntoa(addr.sin_addr)<<":"<<ntohs(addr.sin_port)<<std::endl;

			delete ptcp;
		}
};


int main()
{
	net::io_server *pio_server = new net::io_server;
	pio_server->init(3);

	Client client(pio_server,"127.0.0.1",5555);

	pio_server->run();
}
