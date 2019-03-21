#include <arpa/inet.h>
#include <unistd.h>
#include "mc_tcp.h"

#define		LISTEN_EVENTS	512


using net::mc_tcp;

mc_tcp::mc_tcp(io_server *pio_server,net::TYPE type):m_io_server(pio_server),m_type(type)
{
	m_ptcp = NULL;
	if(m_type == net::NORMAL)
		m_status = net::CLOSED;
}

mc_tcp::mc_tcp(io_server *pio_server,const char *ip,uint16_t port,net::TYPE type):m_io_server(pio_server),m_type(type)
{
	if(m_type == net::LISTEN){
		m_ori_addr.sin_family = AF_INET;
		m_ori_addr.sin_port = htons(port);
		m_ori_addr.sin_addr.s_addr = inet_addr(ip);

		server_create();
	}
	else {
		m_dst_addr.sin_family = AF_INET;
		m_dst_addr.sin_port = htons(port);
		m_dst_addr.sin_addr.s_addr = inet_addr(ip);
	}
		
	m_ptcp = NULL;
}

mc_tcp::mc_tcp(io_server *pio_server,struct sockaddr_in &addr,net::TYPE type):m_io_server(pio_server),m_type(type)
{
	if(m_type == net::LISTEN){
		m_ori_addr = addr;
		server_create();
	}
	else 
		m_dst_addr = addr;

	m_ptcp = NULL;
}


mc_tcp::~mc_tcp()
{
}

void mc_tcp::async_accept(mc_tcp * ptcp,callback_cb cb)
{
	m_cb = cb;	
	if(m_type == net::LISTEN){
		m_ptcp = ptcp;
		m_io_server->add_event_msg(this,net::EVENT_ACCEPT);
	}
}

void mc_tcp::async_connect(callback_cb cb)
{
	m_cb = cb;
	m_io_server->add_event_msg(this,net::EVENT_CONNECT);
}

void mc_tcp::async_read(std::shared_ptr<buffer> pbuffer,callback_cb cb)
{
	m_buffer = pbuffer;
	m_cb = cb;
	m_io_server->add_event_msg(this,net::EVENT_READ);
}

void mc_tcp::async_write(std::shared_ptr<buffer> pbuffer,callback_cb cb)
{
	m_buffer = pbuffer;
	m_writed_len = 0;
	m_cb = cb;
	m_io_server->add_event_msg(this,net::EVENT_WRITE);
}

void mc_tcp::async_close(callback_cb cb)
{
	m_cb = cb;
	m_io_server->add_event_msg(this,net::EVENT_CLOSE);
}

void mc_tcp::set_ori_addr(struct sockaddr_in &addr)
{
	m_ori_addr = addr;
}

void mc_tcp::set_dst_addr(struct sockaddr_in &addr)
{
	m_dst_addr = addr;
}

void mc_tcp::set_client_ori_addr(struct sockaddr_in &addr)
{
	m_ptcp->set_ori_addr(addr);
}

void mc_tcp::set_client_dst_addr(struct sockaddr_in &addr)
{
	m_ptcp->set_dst_addr(addr);
}

void mc_tcp::set_client_fd(int32_t fd)
{
	m_ptcp->set_socket_fd(fd);
	m_ptcp->set_socket_status(net::CONNECTED);
}

int32_t mc_tcp::get_client_fd()
{
	return m_ptcp->get_socket_fd();
}

void mc_tcp::set_socket_status(net::STATUS status)
{
	m_status = status;
}

void mc_tcp::set_socket_fd(int32_t fd)
{
	m_fd = fd;
}

int32_t mc_tcp::get_socket_fd()
{
	return m_fd;
}

char *mc_tcp::get_read_buffer()
{
	return m_buffer->get();
}

int32_t mc_tcp::get_read_buffer_len()
{
	return m_buffer->get_len();
}

void mc_tcp::set_readed_len(uint32_t len)
{
	m_buffer->set_len(len);
}

char *mc_tcp::get_write_buffer()
{
	return m_buffer->get();
}

int32_t mc_tcp::get_write_buffer_len()
{
	return m_buffer->get_len();
}

char *mc_tcp::get_unwrited_buffer()
{
	return m_buffer->get() + m_writed_len;
}

int32_t mc_tcp::get_unwrited_buffer_len()
{
	return m_buffer->get_len() - m_writed_len;
}

void mc_tcp::set_writed_len(int32_t len)
{
	m_writed_len += len;
}

net::TYPE mc_tcp::get_type()
{
	return m_type;
}

net::STATUS mc_tcp::get_status()
{
	return m_status;
}

void mc_tcp::callback_function(error_code &code,int32_t event)
{		
	if(event == EVENT_ACCEPT)
		m_cb(code,m_ptcp);
	else 
		m_cb(code,this);  
	/*
	switch(event)
	{
		case net::EVENT_ACCEPT:
			m_cb(code,m_ptcp);
			break;
		case net::EVENT_READ:
		case net::EVENT_WRITE:
		case net::EVENT_CONNECT:
		case net::EVENT_CLOSE:
			m_cb(code,this);
			break;
	}
	*/
}

struct sockaddr_in mc_tcp::get_ori_addr()
{
	return m_ori_addr;
}

struct sockaddr_in mc_tcp::get_dst_addr()
{
	return m_dst_addr;
}

void mc_tcp::close_fd()
{
	close(m_fd);
}

int32_t mc_tcp::server_create()
{
	int32_t   on = 1;

	m_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(m_fd < 0)
		return m_fd;

	setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	int32_t ret = bind(m_fd,(struct sockaddr *)&m_ori_addr,sizeof(struct sockaddr_in));
	if( ret < 0){
		close(m_fd);
		return ret;
	}
	ret = listen(m_fd,LISTEN_EVENTS);
	if (ret < 0){
		close(m_fd);
		return ret;
	}

	return 0;
}


