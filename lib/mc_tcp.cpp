#include <arpa/inet.h>
#include <unistd.h>
#include "mc_tcp.h"

#define		LISTEN_EVENTS	512


using net::mc_tcp;

mc_tcp::mc_tcp(io_service *pio_service,net::TYPE type):m_io_service(pio_service),m_type(type)
{
	init();
	if(m_type == net::NORMAL)
		m_status = net::CLOSED;
}

mc_tcp::mc_tcp(io_service *pio_service,end_point ep,net::TYPE type):m_io_service(pio_service),m_type(type)
{
	init();
	if(m_type == net::LISTEN){
		m_ori_addr = ep;

		server_create();
	}
	else {
		m_dst_addr = ep;
	}
}

mc_tcp::mc_tcp(io_service *pio_service,end_point &ep,net::TYPE type):m_io_service(pio_service),m_type(type)
{
	init();
	if(m_type == net::LISTEN){
		m_ori_addr = ep;

		server_create();
	}
	else {
		m_dst_addr = ep;
	}
}

mc_tcp::~mc_tcp()
{
}

void mc_tcp::init()
{
	m_ptcp = NULL;
	m_event = 0;
}

void mc_tcp::async_accept(mc_tcp * ptcp,callback_cb cb)
{
	m_cb[EVENT_ACCEPT] = cb;	
	if(m_type == net::LISTEN){
		m_ptcp = ptcp;
		m_io_service->add_event_msg(this,net::EVENT_ACCEPT);
	}
}

void mc_tcp::async_connect(callback_cb cb)
{
	m_cb[EVENT_CONNECT] = cb;
	m_io_service->add_event_msg(this,net::EVENT_CONNECT);
}

void mc_tcp::async_connect(net::end_point &ep,callback_cb cb)
{
	m_dst_addr = ep;
	m_cb[EVENT_CONNECT] = cb;
	m_io_service->add_event_msg(this,net::EVENT_CONNECT);
}

void mc_tcp::async_read(std::shared_ptr<buffer> pbuffer,callback_cb cb)
{
	m_read_buffer = pbuffer;
	m_cb[EVENT_READ] = cb;
	m_io_service->add_event_msg(this,net::EVENT_READ);
}

void mc_tcp::async_write(std::shared_ptr<buffer> pbuffer,callback_cb cb)
{
	m_write_buffer = pbuffer;
	m_writed_len = 0;
	m_cb[EVENT_WRITE] = cb;
	m_io_service->add_event_msg(this,net::EVENT_WRITE);
}

void mc_tcp::async_close(callback_cb cb)
{
	m_cb[EVENT_CLOSE] = cb;
	m_io_service->add_event_msg(this,net::EVENT_CLOSE);
}

void mc_tcp::set_ori_addr(end_point addr)
{
	m_ori_addr = addr;
}

void mc_tcp::set_dst_addr(end_point addr)
{
	m_dst_addr = addr;
}

void mc_tcp::set_client_ori_addr(end_point addr)
{
	m_ptcp->set_ori_addr(addr);
}

void mc_tcp::set_client_dst_addr(end_point addr)
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

uint8_t *mc_tcp::get_read_buffer()
{
	return m_read_buffer->get();
}

const int32_t mc_tcp::get_read_buffer_len()
{
	return m_read_buffer->get_len();
}

void mc_tcp::set_readed_len(uint32_t len)
{
	m_read_buffer->set_len(len);
}

const uint8_t *mc_tcp::get_write_buffer()
{
	return m_write_buffer->get();
}

const int32_t mc_tcp::get_write_buffer_len()
{
	return m_write_buffer->get_len();
}

const uint8_t *mc_tcp::get_unwrited_buffer()
{
	return m_write_buffer->get() + m_writed_len;
}

const int32_t mc_tcp::get_unwrited_buffer_len()
{
	return m_write_buffer->get_len() - m_writed_len;
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

bool mc_tcp::is_open()
{
	return m_status == net::CONNECTED;
}

bool mc_tcp::is_close()
{
	return m_status == net::CLOSED;
}

uint32_t mc_tcp::get_event()
{
	return m_event;
}

void mc_tcp::reset_event()
{
	m_event = 0;
}

void mc_tcp::add_event(uint32_t e)
{
	m_event |= e;
}

void mc_tcp::del_event(uint32_t e)
{
	m_event &= ~e;
}

void mc_tcp::callback_function(error_code &code,int32_t event)
{		
	callback_cb cb = m_cb[event];
	m_cb[event] = nullptr;
	if(event == EVENT_CLOSE)
	{
		m_cb[EVENT_READ] = nullptr;
		m_cb[EVENT_WRITE] = nullptr;
		m_cb[EVENT_CONNECT] = nullptr;
		m_cb[EVENT_CLOSE] = nullptr;
	}

	if(cb){
		if(event == EVENT_ACCEPT)
			cb(code,m_ptcp);
		else 
			cb(code,this);  
	}
}

net::end_point mc_tcp::get_ori_addr()
{
	return m_ori_addr;
}

net::end_point mc_tcp::get_dst_addr()
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

	struct sockaddr_in addr = m_ori_addr.get();
	int32_t ret = bind(m_fd,(struct sockaddr *)&addr,sizeof(addr));
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


