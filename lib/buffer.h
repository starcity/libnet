#ifndef BUFFER_H
#define	BUFFER_H
#include <stdint.h>

namespace net
{
	class buffer
	{
		public:

		/*	buffer(int32_t max_size = 4096)
			{
				m_buf = new char[max_size];
				if(NULL != m_buf)
					m_len = max_size;
				else 
					m_len = 0;
			}

		
*/
/*			buffer(const uint8_t *buf,int32_t size):m_cbuf(buf),m_len(size)
			{
			}
*/
			buffer(uint8_t *buf,int32_t size):m_buf(buf),m_len(size)
			{
			}

			~buffer()
			{
			//	if(m_buf != NULL)
			//		delete [] m_buf;
			}

			uint8_t *get() 
			{
				return m_buf;
			}

/*			const uint8_t *get_const()
			{
				return m_cbuf;
			}
*/
			int32_t get_len()
			{
				return m_len;
			}

			void set_len(int32_t size)
			{
				m_len = size;
			}

		private:
//			const uint8_t  *m_cbuf;
			uint8_t		   *m_buf;
			int32_t		    m_len;
	};
}
#endif
