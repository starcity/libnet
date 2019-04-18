#include <sys/ioctl.h>
#include <sys/epoll.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "io_service.h"
#include "error_code.h"

#define	  MAX_EPOLL_SIZE		512

using net::io_service;

io_service::io_service()
{
	m_running = true;
	m_nthread = 0;
}

io_service::~io_service()
{
	for(auto t:m_array_thread)
		t->join();
}

int32_t io_service::init(int32_t nthread)
{
	m_epfd = epoll_create(MAX_EPOLL_SIZE);
	if( m_epfd < 0 )
		return m_epfd;

	m_nthread = nthread;

	for( int32_t i = 0 ; i < nthread;i ++){
		thread *t(new thread(&io_service::task_contention,this));
		m_array_thread.push_back(t);
	}
	return FUNCTION_SUCESSED;
}

void io_service::stop()
{
	m_running = false;
}

void io_service::task_contention()
{
	event_task task;                                                        
	if(0  == m_nthread){
		if(!m_list_task.empty()){ 
				task = m_list_task.front();                                                             
				m_list_task.pop_front();  
				task.psock->callback_function(task.code,task.event);
		}
	}
	else {
		while(m_running){                                                                                       
			std::unique_lock<std::mutex> lk(m_mutex);                                                          
			while(m_list_task.empty()){                                                                        
				m_cond.wait(lk);                                                                             
				if(!m_running)                                                                                
					break;                                                                           
			}                                                                                                           
			if(!m_list_task.empty()){                                                                             
				task = m_list_task.front();                                                             
				m_list_task.pop_front();                                                                      
				lk.unlock();                                                                                 
				task.psock->callback_function(task.code,task.event);
			}                                      
		}
	}
}

void io_service::handle_epoll()
{  
	int32_t nfds;
	struct epoll_event	events[MAX_EPOLL_SIZE];

	nfds = epoll_wait(m_epfd,events,MAX_EPOLL_SIZE,0);  
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

void io_service::run()
{
	while(m_running){
		handle_event_msg();
		handle_epoll();
		if(0 == m_nthread)
			task_contention();
	}
}

void io_service::set_task(event_task &task)
{
	if(0 == m_nthread){
		m_list_task.push_back(task);  
	}
	else {
		std::unique_lock<std::mutex> lk(m_mutex); 
		m_list_task.push_back(task);  
		m_cond.notify_one();       
		lk.unlock();  
	}
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

void io_service::set_socket_status(base_socket *psock,int32_t ret,event_task &task)
{
	if(ret > 0 ){
		task.code.set_ret(FUNCTION_SUCESSED);
		psock->set_readed_len(ret);
	}
	else if(ret == 0){
		task.code.set_ret(FUNCITON_SOCKET_PEER_CLOSE);
		psock->set_socket_status(net::CLOSED);
	}
	else {

		if((errno != EINTR) && (errno != EWOULDBLOCK) && (errno != EAGAIN)){
			task.code.set_ret(FUNCITON_SOCKET_PEER_CLOSE);
			psock->set_socket_status(net::CLOSED);
		}
		else 
			task.code.set_ret(FUNCTION_RETRY);
	}
}

void io_service::set_nonblock(int32_t fd)
{
	int32_t on = 1;
	ioctl(fd,FIONBIO,&on);
}

void io_service::add_event_msg(base_socket *psock,net::SOCKET_EVENT event)
{
	event_msg msg;
	msg.psock = psock;
	msg.event = event;

	if(0 == m_nthread)
		m_list_msg.push_back(msg);
	else {
		m_event_mutex.lock();
		m_list_msg.push_back(msg);
		m_event_mutex.unlock();
	}
}

void io_service::handle_event_msg()
{
	if(m_list_msg.empty())
		return;

	event_msg msg;
	if(0 == m_nthread){
		msg = m_list_msg.front();
		m_list_msg.pop_front();
	}
	else {
		m_event_mutex.lock();
		msg = m_list_msg.front();
		m_list_msg.pop_front();
		m_event_mutex.unlock();
	}

	switch(msg.event)
	{
		case net::EVENT_ACCEPT:
		case net::EVENT_READ:
			add_epoll_ctl(EPOLLIN,msg.psock);
			break;
		case net::EVENT_WRITE:
			add_epoll_ctl(EPOLLOUT,msg.psock);
			break;
		case net::EVENT_CONNECT:
			{
				int32_t ret = event_connect(msg.psock);
				if(ret <= 0){
					event_task task;

					task.psock = msg.psock;
					task.event = net::EVENT_CONNECT;
					task.code.set_code(ret,errno);
					set_task(task);
				}
				else 
					add_epoll_ctl(EPOLLOUT,msg.psock);
			}
			break;
		case net::EVENT_CLOSE:
			{
				event_task task;
				task.psock = msg.psock;
				task.event = net::EVENT_CLOSE;

				task.code.set_code(FUNCTION_SUCESSED,errno);

				epoll_ctl(m_epfd,EPOLL_CTL_DEL,msg.psock->get_socket_fd(),NULL);

				msg.psock->reset_event();
				msg.psock->close_fd();
				set_task(task);
			}
			break;
	}
}

int32_t io_service::event_connect(base_socket *psock)
{
	int ret = FUNCTION_SUCESSED;
	int32_t fd = socket(AF_INET,SOCK_STREAM,0);
	if(fd < 0)return FUNCTION_SOCKET_FAILED;

	psock->set_socket_fd(fd);

	set_nonblock(fd);

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
	event_task task; 

	int32_t fd = accept(psock->get_socket_fd(),NULL,NULL);
	if( fd > 0 ){
		task.code.set_ret(FUNCTION_SUCESSED);
		psock->set_client_fd(fd);
		set_socket_addr(psock,true);
		set_nonblock(fd);
	}
	else 
		task.code.set_ret(FUNCITON_ACCEPT_FAILED);

	task.code.set_errno(errno);
	task.psock = psock;
	task.event = net::EVENT_ACCEPT;

	set_task(task);

	epoll_ctl(m_epfd,EPOLL_CTL_DEL,psock->get_socket_fd(),NULL);
	psock->del_event(EPOLLIN);
}

void io_service::handle_read(base_socket *psock)
{
	event_task task;                                                                     
	int32_t len = psock->get_read_buffer_len();
	int32_t fd = psock->get_socket_fd();

	int32_t ret = recv(fd,psock->get_read_buffer(),len,0);

	set_socket_status(psock,ret,task);

	task.code.set_errno(errno);
	task.psock = psock;
	task.event = net::EVENT_READ;                                                                
	set_task(task);

	uint32_t ev = psock->get_event();
	if(ev == EPOLLIN){
		epoll_ctl(m_epfd,EPOLL_CTL_DEL,fd,NULL);
	}
	else {
		struct epoll_event event;
		event.data.ptr = static_cast<void *>(psock);
		event.events = (ev & (~EPOLLIN));
		epoll_ctl(m_epfd,EPOLL_CTL_MOD,psock->get_socket_fd(),&event);
	}

	psock->del_event(EPOLLIN);
}

void io_service::handle_write(base_socket *psock,uint32_t event)
{
	event_task task;                                                                     
	task.psock = psock;
	int32_t fd = psock->get_socket_fd();
	if(psock->is_open()){
		if(psock->get_unwrited_buffer_len()){
			int32_t	ret = send(fd,psock->get_unwrited_buffer(),psock->get_unwrited_buffer_len(),0); 
			if(ret != psock->get_unwrited_buffer_len()){
				psock->set_writed_len(ret);
				return;
			}
			
			task.code.set_ret(FUNCTION_SUCESSED);
	
			task.event = net::EVENT_WRITE;   
		}
	}
	else {
		if(event == EPOLLOUT){
			task.code.set_ret(FUNCTION_SUCESSED);
			psock->set_socket_status(net::CONNECTED);
			set_socket_addr(psock,false);
		}
		else {
			task.code.set_ret(FUNCTION_CONNECT_FAILED);
			psock->close_fd();
		}

		task.code.set_errno(errno);
		task.event = net::EVENT_CONNECT;
	}

	uint32_t ev = psock->get_event();
	if(ev == EPOLLOUT){
		epoll_ctl(m_epfd,EPOLL_CTL_DEL,fd,NULL);
	}
	else {
		struct epoll_event event;
		event.data.ptr = static_cast<void *>(psock);
		event.events = (ev & (~EPOLLOUT));
		epoll_ctl(m_epfd,EPOLL_CTL_MOD,psock->get_socket_fd(),&event);
	}
	psock->del_event(EPOLLOUT);

	set_task(task);
}

void io_service::add_epoll_ctl(uint32_t e,base_socket *psock)
{
	struct epoll_event event;

	event.data.ptr = static_cast<void *>(psock);

	uint32_t ev = psock->get_event();
	if(ev){
		e |= ev;
		event.events = e;
		epoll_ctl(m_epfd,EPOLL_CTL_MOD,psock->get_socket_fd(),&event);
	}
	else {
		event.events = e ;
		epoll_ctl(m_epfd,EPOLL_CTL_ADD,psock->get_socket_fd(),&event);
	}
	psock->add_event(e);
}
