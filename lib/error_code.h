#ifndef   ERROR_CODE_H
#define	  ERROR_CODE_H

/****************Function Internal Return Code********************************/

#define			FUNCTION_SUCESSED					0X00
#define			FUNCTION_RETRY						0x01
#define			FUNCTION_SOCKET_FAILED			   -0x01
#define			FUNCTION_CONNECT_FAILED			   -0X10
#define         FUNCTION_CONNECT_WAIT				0X10
#define			FUNCITON_ACCEPT_FAILED			   -0x20
#define			FUNCITON_SOCKET_PEER_CLOSE		   -0x30
#define         FUNCTION_SOCKET_SOCKET_PAIR		   -0X31
#define         FUNCTION_EPOLL_CREATE			   -0X32
#define         FUNCTION_MEMORY_NEW				   -0X33


#endif
