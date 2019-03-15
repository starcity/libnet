#ifndef BUFFER_H
#define	BUFFER_H
#include <stdint.h>

namespace net
{
	class buffer
	{
		public:
			buffer(int32_t max_size = 4096)
			{
				m_buf = new char[max_size];
				m_len = max_size;
			}

			buffer(char *buf,int32_t size)
			{
				m_buf = buf;
				m_len = size;
			}

			~buffer()
			{
				if(m_buf != NULL)
					delete [] m_buf;
			}

			char *get()
			{
				return m_buf;
			}

			int32_t get_len()
			{
				return m_len;
			}

			void    set_len(int32_t size)
			{
				m_len = size;
			}

		private:
			char	   *m_buf;
			int32_t		m_len;
	};
}
#endif
