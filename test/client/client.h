#pragma once
#ifndef	__ASIO_SERVER_H
#define __ASIO_SERVER_H

#include "net.h"
#include <iostream>
#include <string>


using namespace std;


class client
{
public:
	client(string ip,uint16_t port);
	virtual ~client();

	void	start(int32_t thread_number = 0);//0 default -1:System Automation Settings


private:
	void handle_connect(net::error_code &code,net::mc_tcp *ptcp);
	void on_read(net::error_code &code,net::mc_tcp *ptcp);
	void on_write(net::error_code &code,net::mc_tcp *ptcp);
	void on_close(net::error_code &code,net::mc_tcp *ptcp);


private:
	net::io_service		*m_pio_service;
	net::end_point		 m_server_point;
	int32_t				 m_total;
};

#endif
