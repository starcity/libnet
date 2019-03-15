#ifndef NET_H
#define NET_H
#include <memory>
#include <map>
#include <thread>
#include <vector>
#include <list>
#include <mutex>
#include <condition_variable>
#include "base_socket.h"

using namespace std;



namespace net
{
	class io_server
	{
		public:
			io_server();
			~io_server();

			int32_t init(int32_t nthread);
			void	run();

			void    add_event_msg(base_socket *psock,net::SOCKET_EVENT event);

		private:
			void    handle_epoll();
			void    task_contention();
			void	set_task(event_task &task);
			void    set_socket_addr(base_socket *psock,bool is_client);
			void    set_socket_status(base_socket *psock,int32_t ret);
			void	handle_event_msg();
			void	set_nonblock(int32_t fd);
			int32_t event_connect(base_socket *psock);
			void    handle_accept(base_socket *psock);
			void    handle_read(base_socket *psock);
			void    handle_write(base_socket *psock,uint32_t event);

		private:
			bool                m_running;
			int32_t				m_epfd;
			mutex               m_mutex;//use it with cond together 
			mutex               m_event_mutex;//use it for m_map_event 
			condition_variable  m_cond;
			vector<thread *>    m_array_thread; 
			list<event_task>    m_list_task;
			list<event_msg>     m_list_msg;
	};
}

#endif
