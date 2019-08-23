#ifndef RING_BUFFER_H
#define RING_BUFFER_H

/*
 *环形缓冲区 无锁队列
 * 只支持一个生产者一个消费者
 */
class Ring_buffer
{
public:
    Ring_buffer(unsigned int total = 2 *1024 *1024)
    {
        if (total & (total - 1)) {
               total = pow_of_two(total);
           }
        size = total;
        buffer = new unsigned char[size];
        in = 0;
        out = 0;
    }

    ~Ring_buffer()
    {
        if( NULL != buffer)
            delete [] buffer;
    }

    //压入数据
    int data_push(unsigned char *data,unsigned int len)
    {
        unsigned int l;
        unsigned int tmp_len = size - in + out;

        len = len > tmp_len?tmp_len:len;

        tmp_len = size - (in & (size - 1)); //& 运算代替% size必须是2的幂
        l = len > tmp_len?tmp_len:len;
        memcpy(buffer + (in & (size -1)),data,l);
        memcpy(buffer,data + l,len - l);

        in += len;
        return len;
    }

    //读出数据
    int data_pull(unsigned char *buf,unsigned int len)
    {
        unsigned int l;
        unsigned int tmp_len = in - out;

        len = len > tmp_len?tmp_len:len;

        tmp_len = size - (out & (size -1));

        l = len > tmp_len?tmp_len:len;
        memcpy(buf,buffer + (out & (size - 1)),l);
        memcpy(buf + l,buffer,len - l);

        out += len;
        return len;
    }

	//读出数据 不删除
    int data_read(unsigned char *buf,unsigned int len)
    {
        unsigned int l;
        unsigned int tmp_len = in - out;

        len = len > tmp_len?tmp_len:len;

        tmp_len = size - (out & (size -1));

        l = len > tmp_len?tmp_len:len;
        memcpy(buf,buffer + (out & (size - 1)),l);
        memcpy(buf + l,buffer,len - l);

        return len;
    }

	//获取缓冲区容量
	int get_capacity()
	{
		return size;
	}

	//获取数据长度
	int get_len()
	{
		return in - out;
	}


private:
    //size最近2的幂
    static unsigned int pow_of_two(unsigned int size)
    {
        unsigned int bit_count = 0;
        while( size >>= 1){
            bit_count ++;
        }
        bit_count ++;
        return (1UL << bit_count);
    }
    unsigned char *buffer;//缓冲区
    unsigned int size;//缓冲区大小
    unsigned int in;//写入标识
    unsigned int out;//读出标识
};

#endif // RING_BUFFER_H
