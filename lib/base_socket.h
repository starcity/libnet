#ifndef BASE_SOCKET_H
#define BASE_SOCKET_H
#include <netinet/in.h>
#include <stdint.h>
#include <memory>
#include <errno.h>
#include <string.h>
#include "error_code.h"
#include "end_point.h"


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
        PEER_CLOSE,
		SERVER,
	};

	enum SOCKET_EVENT
	{
		EVENT_ACCEPT = 0,
		EVENT_READ,
		EVENT_WRITE,
		EVENT_CONNECT,
		EVENT_CLOSE,
	};

	class base_socket;

	typedef struct error_code
	{
		void set_ret(int32_t ret)
		{
			m_ret = ret;
		}

		void set_errno(int32_t err)
		{
			m_err = err;
		}

		void set_code(int32_t ret,int32_t err)
		{
			m_ret = ret;
			m_err = err;
		}

		int32_t value() const 
		{
			return m_ret;
		}

		string message() const
		{
			return string(strerror(m_ret));
		}

		bool operator!() const
		{
			return m_ret == FUNCTION_SUCESSED;
		}

		private:
			int32_t m_ret ;
			int32_t m_err;
	}error_code;


	class base_socket
	{
		public:

			virtual				   ~base_socket(){}
			virtual void			set_socket_fd(int32_t fd) = 0;
			virtual int32_t			get_socket_fd() = 0;
			virtual uint8_t *		get_read_buffer() = 0;
			virtual const int32_t	get_read_buffer_len() = 0;
			virtual void			set_readed_len(uint32_t len) = 0;
			virtual const uint8_t*	get_write_buffer() = 0;
			virtual const int32_t 	get_write_buffer_len() = 0;
			virtual const uint8_t * get_unwrited_buffer() = 0;
			virtual const int32_t 	get_unwrited_buffer_len() = 0;
			virtual void			set_writed_len(int32_t len) = 0;
			virtual enum TYPE		get_type() = 0; 
			virtual enum STATUS     get_status() = 0;
			virtual bool			is_open()    = 0;
			virtual uint32_t		get_event() = 0;
            virtual void            reset_event() = 0;
			virtual void			add_event(uint32_t e) = 0;
			virtual void			del_event(uint32_t e) = 0;
			virtual void			callback_function(error_code &code,int32_t event) = 0;
			virtual end_point		get_ori_addr() = 0;
			virtual end_point		get_dst_addr() = 0; 
			virtual void			set_client_ori_addr(end_point addr) = 0;
			virtual void			set_client_dst_addr(end_point addr) = 0;
			virtual void			set_ori_addr(end_point addr) = 0;
			virtual void			set_dst_addr(end_point addr) = 0;
			virtual void			set_client_fd(int32_t fd) = 0;
			virtual int32_t			get_client_fd() = 0;
			virtual	void			set_socket_status(enum STATUS status) = 0;
			virtual	void			close_fd() = 0;
			virtual	void			set_epoll_fd(int32_t epfd) = 0;
			virtual	int32_t			get_epoll_fd() = 0;
	};
}

#endif
