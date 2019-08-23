#ifndef NET_H
#define NET_H
#include <memory>
#include <vector>
#include <thread>
#include <map>
#include <iostream>
#include <atomic>
#include "base_socket.h"



struct args_thread
{
	int32_t   epfd;
	int32_t  *pair;
};

namespace net
{
	class io_service
	{
		public:
			io_service();
			~io_service();

			//0 default -1:The most suitable thread number
			int32_t init(int32_t thread_number = 0);
			void	run();
			void	stop();

			void    add_event_msg(base_socket *psock,net::SOCKET_EVENT event);
			int32_t get_listen_epoll_fd();
			int32_t get_work_epoll_fd();
			void	increase_epoll_fd(int32_t epfd);
			void	reduction_epoll_fd(int32_t epfd);

		private:
			void    handle_epoll(int32_t epfd);
			void	set_task(base_socket *psock,error_code &code,uint32_t e);
			void    set_socket_addr(base_socket *psock,bool is_client);
			void    set_socket_status(base_socket *psock,int32_t ret,error_code &code);
			void	set_nonblock(int32_t fd);
			int32_t event_connect(base_socket *psock);
			void    handle_accept(base_socket *psock);
			void    handle_read(base_socket *psock);
			void    handle_write(base_socket *psock,uint32_t event);
			void    add_epoll_ctl(uint32_t e,base_socket * psock);

		private:
			bool				  m_running;
			int32_t				  m_epfd;
			int32_t				  m_nthread;
			map<int32_t,std::atomic<int32_t> >  m_epfd_connects;
			vector<std::thread *>	  m_array_thread;
	};
}

#endif
