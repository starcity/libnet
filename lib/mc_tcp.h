#ifndef MC_TCP_H
#define MC_TCP_H
#include "base_socket.h"
#include "io_server.h"
#include "buffer.h"


namespace net
{
	class mc_tcp:public base_socket
	{
		public:
			typedef std::function<void(int32_t ret,mc_tcp *)> callback_cb;
			mc_tcp(io_server *pio_server,net::TYPE type = net::NORMAL);
			mc_tcp(io_server *pio_server,const char *ip,uint16_t port,net::TYPE type = net::NORMAL);
			mc_tcp(io_server *pio_server,struct sockaddr_in &addr,net::TYPE type = net::NORMAL);
			~mc_tcp();


			int32_t			    async_accept(mc_tcp *ptcp,callback_cb cb);// must use in main thread
			void				async_connect(callback_cb cb);
			void 			    async_read(std::shared_ptr<buffer> pbuffer,callback_cb cb);
			void 			    async_write(std::shared_ptr<buffer> pbuffer,callback_cb cb);
			void 			    async_close(callback_cb cb);


			//inherit from base_socket
			virtual void		set_socket_fd(int32_t fd);
			virtual int32_t		get_socket_fd();
			virtual char *		get_read_buffer();
			virtual int32_t		get_read_buffer_len();
			virtual void		set_readed_len(uint32_t len);
			virtual char *		get_write_buffer();
			virtual int32_t 	get_write_buffer_len();
			virtual enum TYPE	get_type(); 
			virtual enum STATUS get_status();
			virtual void	    callback_function(int32_t ret,int32_t event);
			virtual struct sockaddr_in get_ori_addr();
			virtual struct sockaddr_in get_dst_addr(); 
			virtual void		set_ori_addr(struct sockaddr_in &addr);
			virtual void		set_dst_addr(struct sockaddr_in &addr);
			virtual void		set_client_ori_addr(struct sockaddr_in &addr);
			virtual void		set_client_dst_addr(struct sockaddr_in &addr);
			virtual void		set_client_fd(int32_t fd);
			virtual int32_t		get_client_fd();
			virtual	void		set_socket_status(net::STATUS status);


		private:
			net::TYPE					m_type;
			net::STATUS					m_status;
			int32_t						m_fd;
			struct sockaddr_in			m_ori_addr;
			struct sockaddr_in			m_dst_addr;
			callback_cb			    	m_cb;		
			std::shared_ptr<buffer>	    m_buffer;
			int32_t						m_len;
			io_server				   *m_io_server;
			mc_tcp					   *m_ptcp;
	};
}

#endif
