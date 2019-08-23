#include "client.h"
#include <string.h>


#define			MAX_LENGTH			4096

client::client(string ip,uint16_t port):m_pio_service(new net::io_service),m_server_point(ip,port)
{
	m_total = 0;
}

client::~client()
{
	m_pio_service->stop();
}

void client::start(int32_t thread_number)
{
	net::mc_tcp *ptcp = new net::mc_tcp(m_pio_service);
	if(nullptr == ptcp)exit(0);

	ptcp->async_connect(m_server_point,std::bind(&client::handle_connect,this,std::placeholders::_1,std::placeholders::_2));

	int ret = m_pio_service->init(thread_number);
	if(ret < 0 )exit(0);

	m_pio_service->run();
}

void client::handle_connect(net::error_code &code,net::mc_tcp *ptcp)
{
	if(!code){
		net::end_point ep = ptcp->get_ori_addr();
		std::cout<<"connected:"<<ep.ip_to_string()<<":"<<ep.port()<<std::endl;

		uint8_t *pdata = new uint8_t[MAX_LENGTH];
		if(nullptr == pdata)exit(0);
		memset(pdata,0x00,MAX_LENGTH);

		sprintf((char *)pdata,"I love you,but somethings is wrong!");
		ptcp->async_write(make_shared<net::buffer>(pdata,strlen((char *)pdata)),std::bind(&client::on_write,this,std::placeholders::_1,std::placeholders::_2));
		m_total ++;
	}
	else std::cout<<"accept failed"<<std::endl;

}

void client::on_read(net::error_code &code,net::mc_tcp *ptcp)
{
	if(!code){
		net::end_point ep = ptcp->get_ori_addr();
		std::cout<<ep.ip_to_string()<<":"<<ptcp->get_read_buffer()<<std::endl;

		if(m_total < 10000){
		net::mc_tcp *pc = new net::mc_tcp(m_pio_service);
			if(nullptr == ptcp)exit(0);

			pc->async_connect(m_server_point,std::bind(&client::handle_connect,this,std::placeholders::_1,std::placeholders::_2));
		}


	//	ptcp->async_write(make_shared<net::buffer>(ptcp->get_read_buffer(),ptcp->get_read_buffer_len()),std::bind(&client::on_write,this,std::placeholders::_1,std::placeholders::_2));
	}
	else {
		ptcp->async_close(std::bind(&client::on_close,this,std::placeholders::_1,std::placeholders::_2));
	}
}

void client::on_write(net::error_code &code,net::mc_tcp *ptcp)
{
	if(!code){
		uint8_t *pdata = const_cast<uint8_t *>(ptcp->get_write_buffer());
		ptcp->async_read(make_shared<net::buffer>(pdata,MAX_LENGTH),std::bind(&client::on_read,this,std::placeholders::_1,std::placeholders::_2));
	}
	else {
		ptcp->async_close(std::bind(&client::on_close,this,std::placeholders::_1,std::placeholders::_2));
		std::cout<<"on_write failed"<<std::endl;
	}
}
void client::on_close(net::error_code &code,net::mc_tcp *ptcp)
{
	net::end_point ep = ptcp->get_ori_addr();
	std::cout<<"close connect:"<<ep.ip_to_string()<<":"<<ep.port()<<std::endl;

	delete ptcp;
}
