#include <sys/epoll.h> 
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include "io_service.h"
#include "error_code.h"

#define	  MAX_EPOLL_SIZE		512


using net::io_service;

io_service::io_service()
{
	m_running = true;
	m_nthread = 0;
	m_epfd = epoll_create(MAX_EPOLL_SIZE);
	if( m_epfd < 0 )
		exit(1);

//	m_epfd_connects[m_epfd] = 0;
}

io_service::~io_service()
{
	for(auto t:m_array_thread)
		t->join();
}

int32_t io_service::init(int32_t thread_number)
{
	m_nthread = thread_number;
	if(-1 == thread_number){
		m_nthread = thread::hardware_concurrency();
	}

	for(int i = 0;i < m_nthread;i++)
	{
		int32_t epfd = epoll_create(MAX_EPOLL_SIZE);
		if(epfd < 0)
			return FUNCTION_EPOLL_CREATE;

		std::cout<<"epfd:"<<epfd<<std::endl;
		m_epfd_connects[epfd] = 0;

		thread *t(new thread(&io_service::handle_epoll,this,epfd));
		m_array_thread.push_back(t);
	}

	return FUNCTION_SUCESSED;
}

void io_service::stop()
{
	m_running = false;
}

void io_service::handle_epoll(int32_t epfd)
{  
	while(m_running){
		int32_t nfds;
		struct epoll_event	events[MAX_EPOLL_SIZE];

		nfds = epoll_wait(epfd,events,MAX_EPOLL_SIZE,-1);  
		for(int i = 0; i < nfds ; i++ ){
			base_socket *psock = static_cast<base_socket *>(events[i].data.ptr);
			if( psock->get_type() == net::LISTEN){  
				handle_accept(psock);
			} 
			else {
				if( events[i].events & EPOLLIN ){  
					handle_read(psock);
				}  
				else if(events[i].events & EPOLLOUT){  
					handle_write(psock,events[i].events);
				}  
			}  
		}  
	}
} 

void io_service::run()
{
	handle_epoll(m_epfd);
}

void io_service::set_task(base_socket *psock,error_code &code,uint32_t event)
{
	psock->callback_function(code,event);
}

void io_service::set_socket_addr(base_socket *psock,bool is_client)
{
	struct sockaddr_in addr;
	socklen_t    len = sizeof(struct sockaddr_in);

	if(is_client){
		int32_t fd = psock->get_client_fd();
		getsockname(fd, (struct sockaddr *)&addr, &len); 
		psock->set_client_dst_addr(addr);
		len = sizeof(struct sockaddr_in);
		getpeername(fd,(struct sockaddr *)&addr,&len);
		psock->set_client_ori_addr(addr);
	}
	else {
		int32_t fd = psock->get_socket_fd();
		getsockname(fd, (struct sockaddr *)&addr, &len); 
		psock->set_ori_addr(addr);
		len = sizeof(struct sockaddr_in);
		getpeername(fd,(struct sockaddr *)&addr,&len);
		psock->set_dst_addr(addr);
	}
}

void io_service::set_socket_status(base_socket *psock,int32_t ret,error_code &code)
{
	if(ret > 0 ){
		code.set_ret(FUNCTION_SUCESSED);
		psock->set_readed_len(ret);
	}
	else if(ret == 0){
		code.set_ret(FUNCITON_SOCKET_PEER_CLOSE);
		psock->close_fd();
		psock->reset_event();
		psock->add_event(EPOLLIN);
		psock->set_socket_status(net::CLOSED);
	}
	else {

		if((errno != EINTR) && (errno != EWOULDBLOCK) && (errno != EAGAIN)){
			code.set_ret(FUNCITON_SOCKET_PEER_CLOSE);
			psock->close_fd();
			psock->reset_event();
			psock->add_event(EPOLLIN);

			psock->set_socket_status(net::CLOSED);
		}
		else 
			code.set_ret(FUNCTION_RETRY);
	}
}

void io_service::set_nonblock(int32_t fd)
{
	int32_t on = 1;
	ioctl(fd,FIONBIO,&on);
}

void io_service::add_event_msg(base_socket *psock,net::SOCKET_EVENT event)
{
	switch(event)
	{
		case net::EVENT_ACCEPT:
		case net::EVENT_READ:
			add_epoll_ctl(EPOLLIN,psock);
			break;
		case net::EVENT_WRITE:
			add_epoll_ctl(EPOLLOUT,psock);
			break;
		case net::EVENT_CONNECT:
			{
				int32_t ret = event_connect(psock);
				if(ret < 0){
					error_code code;
					code.set_code(ret,errno);

					set_task(psock,code,net::EVENT_CONNECT);
				}
				else 
					add_epoll_ctl(EPOLLOUT,psock);
			}
			break;
		case net::EVENT_CLOSE:
			{
				error_code code;
				code.set_code(FUNCTION_SUCESSED,errno);

				add_epoll_ctl(EPOLLIN,psock);//add epoll when close EPOLLIN event 
				psock->close_fd();
				set_task(psock,code,net::EVENT_CLOSE);
			}
			break;
	}
}

int32_t io_service::get_listen_epoll_fd()
{
	return m_epfd;
}

int32_t io_service::get_work_epoll_fd()
{
	int32_t epfd = -1;
	if(m_nthread){
		int32_t count = m_epfd_connects.begin()->second;
		epfd = m_epfd_connects.begin()->first;
		auto iter = m_epfd_connects.begin();
	std::cout<<"m_epfd_connects fd: "<<iter->first<<std::endl;
		for(iter++ ;iter != m_epfd_connects.end();iter++){
			std::cout<<"m_epfd_connects fd: "<<iter->first<<std::endl;
			if(iter->second < count){
				count = iter->second;
				epfd = iter->first;
			}
		}
	}
	else epfd = m_epfd;

	std::cout<<"get epollfd:"<<epfd<<std::endl;

	return epfd;
}

void io_service::increase_epoll_fd(int32_t epfd)
{
	m_epfd_connects[epfd] ++;
}

void io_service::reduction_epoll_fd(int32_t epfd)
{
	m_epfd_connects[epfd] --;
}

int32_t io_service::event_connect(base_socket *psock)
{
	int ret = FUNCTION_SUCESSED;
	int32_t fd = socket(AF_INET,SOCK_STREAM,0);
	if(fd < 0)return FUNCTION_SOCKET_FAILED;

	psock->set_socket_fd(fd);

	int32_t on = 1;
	ioctl(fd,FIONBIO,&on);

	struct sockaddr_in addr = psock->get_dst_addr().get();
	if(connect(fd,(struct sockaddr *)&addr,sizeof(struct sockaddr_in)) < 0){
		if( errno != EINPROGRESS ){
			ret = FUNCTION_CONNECT_FAILED ;
			psock->close_fd();
		}
		else 
			ret = FUNCTION_CONNECT_WAIT;

	}
	return ret;
}

void io_service::handle_accept(base_socket *psock)
{
	error_code code;

	int32_t fd = accept(psock->get_socket_fd(),NULL,NULL);
	if( fd > 0 ){
		code.set_ret(FUNCTION_SUCESSED);
		psock->set_client_fd(fd);
		set_socket_addr(psock,true);
		set_nonblock(fd);
	}
	else 
		code.set_ret(FUNCITON_ACCEPT_FAILED);

	code.set_errno(errno);

	epoll_ctl(m_epfd,EPOLL_CTL_DEL,psock->get_socket_fd(),NULL);

	psock->del_event(EPOLLIN);

	set_task(psock,code,net::EVENT_ACCEPT);
}

void io_service::handle_read(base_socket *psock)
{
	error_code code;
	int32_t len = psock->get_read_buffer_len();
	int32_t fd = psock->get_socket_fd();
	int32_t ret = recv(fd,psock->get_read_buffer(),len,0);

	set_socket_status(psock,ret,code);

	code.set_errno(errno);

	uint32_t ev = psock->get_event();
	int32_t  epfd = psock->get_epoll_fd();
	if(ev == EPOLLIN){
		epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
		reduction_epoll_fd(epfd);//-- thread epoll fd
	}
	else {
		struct epoll_event event;
		event.data.ptr = static_cast<void *>(psock);
		event.events = (ev & (~EPOLLIN)) ;
		epoll_ctl(epfd,EPOLL_CTL_MOD,psock->get_socket_fd(),&event);
	}

	std::cout<<"in handle_read epfd:"<<epfd<<std::endl;

	psock->del_event(EPOLLIN);

	set_task(psock,code,net::EVENT_READ);
}

void io_service::handle_write(base_socket *psock,uint32_t event)
{
	error_code code;
	uint32_t e;

	int32_t fd = psock->get_socket_fd();
	if(psock->is_open()){
		int32_t	ret = send(fd,psock->get_unwrited_buffer(),psock->get_unwrited_buffer_len(),0); 
		if(ret != psock->get_unwrited_buffer_len()){
			psock->set_writed_len(ret);
			return;
		}
		else {
			code.set_ret(FUNCTION_SUCESSED);

			e = net::EVENT_WRITE;   
		}
	}
	else {
		if(event == EPOLLOUT){
			code.set_ret(FUNCTION_SUCESSED);
			psock->set_socket_status(net::CONNECTED);
			set_socket_addr(psock,false);
		}
		else {
			code.set_ret(FUNCTION_CONNECT_FAILED);
			psock->close_fd();
		}

		code.set_errno(errno);
		e = net::EVENT_CONNECT;
	}

	uint32_t ev = psock->get_event();
	int32_t  epfd = psock->get_epoll_fd();
	if(ev == EPOLLOUT){
		epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
		reduction_epoll_fd(epfd);//-- thread epoll fd
	}
	else {
		struct epoll_event event;
		event.data.ptr = static_cast<void *>(psock);
		event.events = (ev & (~EPOLLOUT)) ;
		epoll_ctl(epfd,EPOLL_CTL_MOD,psock->get_socket_fd(),&event);
	}

	psock->del_event(EPOLLOUT);

	set_task(psock,code,e);
}

void  io_service::add_epoll_ctl(uint32_t e,base_socket * psock)
{
	struct epoll_event event;

	event.data.ptr = static_cast<void *>(psock);

	uint32_t ev = psock->get_event();
	int32_t epfd = psock->get_epoll_fd();
	std::cout<<"add epfd:"<<epfd<<std::endl;
	if(ev){
		e |= ev;
		event.events = e ;
		epoll_ctl(epfd,EPOLL_CTL_MOD,psock->get_socket_fd(),&event);
	}
	else {
		event.events = e ;
		if(epfd != m_epfd)
			increase_epoll_fd(epfd);//increase thread epoll fd
		epoll_ctl(epfd,EPOLL_CTL_ADD,psock->get_socket_fd(),&event);
	}
	psock->add_event(e);
}
