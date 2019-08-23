#include "asio_server.h"


#define			MAX_LENGTH			4096

asio_server::asio_server(string ip,uint16_t port):m_pio_service(new net::io_service),m_ip(ip),m_port(port)
{
}

asio_server::~asio_server()
{
	m_pio_service->stop();
}

void asio_server::start(int32_t thread_number)
{
	m_pserver_tcp = new net::mc_tcp(m_pio_service,net::end_point(m_ip,m_port),net::LISTEN);
	if(nullptr == m_pserver_tcp)exit(0);

	net::mc_tcp *tcp_client = new net::mc_tcp(m_pio_service); 
	if(nullptr == tcp_client)exit(0);         
	m_pserver_tcp->async_accept(tcp_client,std::bind(&asio_server::handle_accept,this,std::placeholders::_1,std::placeholders::_2));

	int ret = m_pio_service->init(thread_number);
	if(ret < 0 )exit(0);

	m_pio_service->run();
}

void asio_server::handle_accept(net::error_code &code,net::mc_tcp *ptcp)
{
	if(!code){
		net::end_point ep = ptcp->get_ori_addr();
		std::cout<<"new connect:"<<ep.ip_to_string()<<":"<<ep.port()<<std::endl;

		Ring_buffer *buf = new Ring_buffer;
		if(nullptr != buf){
			m_mutex.locak();
			m_map_data.insert(make_pair<ptcp,buf>);
			m_mutex.unlocak();
		}
		else std::cout<<"new Ring_buffer null"<<std::endl;

		uint8_t *pdata = new uint8_t[MAX_LENGTH];
		if(nullptr == pdata)exit(0);
		ptcp->async_read(make_shared<net::buffer>(pdata,MAX_LENGTH),std::bind(&asio_server::on_read,this,std::placeholders::_1,std::placeholders::_2));

		net::mc_tcp *tcp_client = new net::mc_tcp(m_pio_service); 
		m_pserver_tcp->async_accept(tcp_client,std::bind(&asio_server::handle_accept,this,std::placeholders::_1,std::placeholders::_2));
	}
	else std::cout<<"accept failed"<<std::endl;

}

void asio_server::on_read(net::error_code &code,net::mc_tcp *ptcp)
{
	if(!code){
		data_dealwith(ptcp,ptcp->get_read_buffer(),ptcp->get_read_buffer_len());
		ptcp->async_read(make_shared<net::buffer>(ptcp->get_read_buffer(),MAX_LENGTH),std::bind(&asio_server::on_read,this,std::placeholders::_1,std::placeholders::_2));
//		ptcp->async_write(make_shared<net::buffer>(ptcp->get_read_buffer(),ptcp->get_read_buffer_len()),std::bind(&asio_server::on_write,this,std::placeholders::_1,std::placeholders::_2));
	}
	else {
		ptcp->async_close(std::bind(&asio_server::on_close,this,std::placeholders::_1,std::placeholders::_2));
	}
}

void asio_server::on_write(net::error_code &code,net::mc_tcp *ptcp)
{
	if(!code){
		uint8_t *pdata = ptcp->get_read_buffer();
		ptcp->async_read(make_shared<net::buffer>(pdata,MAX_LENGTH),std::bind(&asio_server::on_read,this,std::placeholders::_1,std::placeholders::_2));
	}
	else {
		ptcp->async_close(std::bind(&asio_server::on_close,this,std::placeholders::_1,std::placeholders::_2));
		std::cout<<"on_write failed"<<std::endl;
	}
}
void asio_server::on_close(net::error_code &code,net::mc_tcp *ptcp)
{
	net::end_point ep = ptcp->get_ori_addr();
	std::cout<<"close connect:"<<ep.ip_to_string()<<":"<<ep.port()<<std::endl;

	m_mutex.locak();
	auto iter = m_map_data.find(ptcp);
	if(iter != m_map_data.end())
		m_map_data.erase(iter);
	m_mutex.unlocak();

	delete ptcp;
}

void asio_server::data_dealwith(net::mc_tcp *ptcp,const uint8_t *pdata,int32_t len)
{
	m_mutex.locak();
	auto iter = m_map_data.find(ptcp);
	m_mutex.unlocak();
	if(iter == m_map_data.end())
		return;

    Ring_buffer *buf = iter->second;
	buf->data_push(pdata,len);

	protocol pro;

	buf->data_read((uint8_t *)&pro,sizeof(protocol));
	int len = buf->get_len();
	if(len < pro.len)
		return ;

	int data_len = pro.len;
	char *data = new char(data_len +1);
	if(nullptr == data){
		std::cout<<"new data failed"<<std::endl;
		return;
	}
	memset(data,0x00,data_len +1);
	buf->data_pull(data,data_len);
	switch(pro.cmd)
	{
		case 1:
			do_file(ptcp,data,data_len);
			break;
		case 2:
			do_other(ptcp,data,data_len);
			break;
		default:
			std::cout<<"cmd wrong"<<std::endl;
	}
	delete [] data;
}

void asio_server::do_file(net::mc_tcp *ptcp,const char *data,int len)
{
	uint8_t type = data[0];

	if(type == 1){
		FILE *pfd = fopen(data+1,"w+");
		if(fd < 0){
			std::cout<<"fopen failed"<<std::endl;
			return;
		}
		m_map_fd[ptcp] = pfd;
		std::cout<<"fopen sucessed"<<std::endl;
	}
	else if(type == 2){
		auto iter = m_map_fd.find(ptcp);
		if(iter == m_map_fd.end())
			return;

		int ret = fwrite(data+1,1,len -1,iter->second);
		if(ret != len - 1)
			std::cout<<"fwrite wrong"<<std::endl;
	}
	else if(type == 3){
		auto iter = m_map_fd.find(ptcp);    
		if(iter == m_map_fd.end()) 
			return;
		fclose(iter->second);
		std::cout<<"fclose sucessed"<<std::endl;
		m_map_fd.erase(iter);
	}
}

void do_other(net::mc_tcp *ptcp,const char *data,int len)
{

}
