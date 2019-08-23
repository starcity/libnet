#pragma once
#ifndef	__ASIO_SERVER_H
#define __ASIO_SERVER_H

#include "ring_buffer.h"
#include "net.h"
#include <iostream>
#include <string>
#include <map>
#include <mutex>


using namespace std;


#pragma pack(1)
struct protocol
{
	uint8_t		 cmd;
	uint16_t	 len;
};
#pragma pack()




class asio_server
{
public:
	asio_server(string ip,uint16_t port);
	virtual ~asio_server();

	void	start(int32_t thread_number = 0);//0 default -1:System Automation Settings


private:
	void handle_accept(net::error_code &code,net::mc_tcp *ptcp);
	void on_read(net::error_code &code,net::mc_tcp *ptcp);
	void on_write(net::error_code &code,net::mc_tcp *ptcp);
	void on_close(net::error_code &code,net::mc_tcp *ptcp);

	void data_dealwith(const uint8_t *pdata,int32_t len);

	void do_file(net::mc_tcp *ptcp,const char *data,int len);
	void do_other(net::mc_tcp *ptcp,const char *data,int len);


private:
	net::mc_tcp			*m_pserver_tcp;
	net::io_service		*m_pio_service;
	string			  	 m_ip;
	uint16_t			 m_port;
	map<net::mc_tcp *,Ring_buffer *> m_map_data;
	map<net::mc_tcp *,FILE *> m_map_fd;
	mutex				 m_mutex;
};

#endif
