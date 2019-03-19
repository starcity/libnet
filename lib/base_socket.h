#ifndef BASE_SOCKET_H
#define BASE_SOCKET_H
#include <stdint.h>
#include <memory>
#include <netinet/in.h>


using namespace std;

namespace net
{
	enum TYPE
	{
		LISTEN = 1,
		NORMAL,
	};
	enum STATUS
	{
		CONNECTED = 1,
		CLOSED,
	};

	enum SOCKET_EVENT
	{
		EVENT_ACCEPT = 1,
		EVENT_READ,
		EVENT_WRITE,
		EVENT_CONNECT,
		EVENT_CLOSE,
	};

	class base_socket
	{
		public:

			virtual			   ~base_socket(){}
			virtual void		set_socket_fd(int32_t fd) = 0;
			virtual int32_t		get_socket_fd() = 0;
			virtual char *		get_read_buffer() = 0;
			virtual int32_t		get_read_buffer_len() = 0;
			virtual void		set_readed_len(uint32_t len) = 0;
			virtual char *		get_write_buffer() = 0;
			virtual int32_t 	get_write_buffer_len() = 0;
			virtual char *      get_unwrited_buffer() = 0;
			virtual int32_t 	get_unwrited_buffer_len() = 0;
			virtual void		set_writed_len(int32_t len) = 0;
			virtual enum TYPE	get_type() = 0; 
			virtual enum STATUS get_status() = 0;
			virtual void	    callback_function(int32_t ret,int32_t event) = 0;
			virtual struct sockaddr_in get_ori_addr() = 0;
			virtual struct sockaddr_in get_dst_addr() = 0; 
			virtual void		set_client_ori_addr(struct sockaddr_in &addr) = 0;
			virtual void		set_client_dst_addr(struct sockaddr_in &addr) = 0;
			virtual void		set_ori_addr(struct sockaddr_in &addr) = 0;
			virtual void		set_dst_addr(struct sockaddr_in &addr) = 0;
			virtual void		set_client_fd(int32_t fd) = 0;
			virtual int32_t		get_client_fd() = 0;
			virtual	void		set_socket_status(enum STATUS status) = 0;
			virtual	void		close_fd() = 0;
	};


	typedef struct event_msg
	{
		enum SOCKET_EVENT  event;
		base_socket		  *psock;
	}event_msg;


	typedef struct event_task
	{
		int32_t			   nret;
		int32_t			   event;
		base_socket       *psock;
	}event_task;

}

#endif
