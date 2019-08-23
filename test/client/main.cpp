#include "client.h"



int main(int argc,char *argv[])
{
	client c("127.0.0.1",5555);
	c.start(0);
}
