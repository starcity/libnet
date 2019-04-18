#ifndef   END_POINT_H
#define   END_POINT_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <string>


using namespace std;

namespace net
{

	class end_point
	{
		public:
			end_point()
			{
			}

			end_point(const string &ip,uint32_t port)
			{
				m_addr.sin_family = AF_INET;
				m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
				m_addr.sin_port = htons(port);
			}

			end_point(const end_point &ep)
			{
				m_addr = ep.m_addr;
			}

			end_point(const struct sockaddr_in &addr)
			{
				m_addr = addr;
			}

			struct sockaddr_in get()
			{
				return m_addr;
			}

			string ip_to_string()
			{
				return inet_ntoa(m_addr.sin_addr);
			}

			uint32_t ip_to_big()
			{
				return m_addr.sin_addr.s_addr;
			}

			uint32_t ip_to_small()
			{
				return ntohl(m_addr.sin_addr.s_addr);
			}

			uint16_t port()
			{
				return ntohs(m_addr.sin_port);
			}

			end_point& operator= (const struct sockaddr_in &addr)
			{
				m_addr = addr;
				return *this;
			}

			end_point& operator= (const end_point &ep)
			{
				m_addr = ep.m_addr;
				return *this;
			}

		private:
			struct sockaddr_in m_addr;
	};


}


#endif
