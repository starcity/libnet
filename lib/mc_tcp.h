#ifndef MC_TCP_H
#define MC_TCP_H
#include <memory>
#include <functional>
#include <iostream>
#include "base_socket.h"
#include "io_service.h"
#include "buffer.h"


namespace net
{
	class mc_tcp:public base_socket
	{
		public:
			typedef std::function<void(error_code &code,mc_tcp *ptcp)> callback_cb;
			mc_tcp(io_service *pio_service,net::TYPE type = net::NORMAL);
			mc_tcp(io_service *pio_service,end_point &ep,net::TYPE type = net::NORMAL);
			mc_tcp(io_service *pio_service,end_point ep,net::TYPE type = net::NORMAL);
			~mc_tcp();


			void					async_accept(mc_tcp *ptcp,callback_cb);// must use in main thread
			void					async_connect(end_point &ep,callback_cb cb);
			void					async_read(std::shared_ptr<buffer> pbuffer,callback_cb cb);
			void					async_write(std::shared_ptr<buffer> pbuffer,callback_cb cb);
			void					async_close(callback_cb cb);

			bool					is_close();


			//inherit from base_socket
			virtual void			set_socket_fd(int32_t fd);
			virtual int32_t			get_socket_fd();
			virtual uint8_t *		get_read_buffer();
			virtual const int32_t	get_read_buffer_len();
			virtual void			set_readed_len(uint32_t len);
			virtual const uint8_t *	get_write_buffer();
			virtual const int32_t	get_write_buffer_len();
			virtual const uint8_t *	get_unwrited_buffer();
			virtual const int32_t	get_unwrited_buffer_len();
			virtual void			set_writed_len(int32_t len);
			virtual enum TYPE		get_type(); 
			virtual enum STATUS		get_status();
			virtual bool			is_open();
			virtual uint32_t		get_event();
			virtual void            reset_event();
			virtual void			add_event(uint32_t e);
			virtual void			del_event(uint32_t e);
			virtual void			callback_function(error_code &code,int32_t event);
			virtual end_point		get_ori_addr();
			virtual end_point		get_dst_addr(); 
			virtual void			set_ori_addr(end_point ep);
			virtual void			set_dst_addr(end_point ep);
			virtual void			set_client_ori_addr(end_point ep);
			virtual void			set_client_dst_addr(end_point ep);
			virtual void			set_client_fd(int32_t fd);
			virtual int32_t			get_client_fd();
			virtual	void			set_socket_status(net::STATUS status);
			virtual void			close_fd();
			virtual void			set_epoll_fd(int32_t epfd);
			virtual int32_t			get_epoll_fd();

		private:
			int32_t					server_create();
			void					init();


		private:
			net::STATUS					m_status;
			int32_t						m_fd;
			end_point					m_ori_addr;
			end_point					m_dst_addr;
			std::shared_ptr<buffer>     m_read_buffer;
			std::shared_ptr<buffer>     m_write_buffer;
			int32_t						m_writed_len;
			io_service			       *m_io_service;
			mc_tcp					   *m_ptcp;
			net::TYPE					m_type;
			callback_cb					m_cb[5];
			uint32_t					m_event;
			int32_t						m_epfd;
	};
}

#endif
