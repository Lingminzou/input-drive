/*
 *   drive_test.c
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>

/* debug message --------------------------*/

#define pinfo(fmt, arg...)   printf("dev info: " fmt, ##arg)

#define pdebug(fmt, arg...)  printf("dev debug: " fmt, ##arg)

#define pwarn(fmt, arg...)   printf("dev warn: " fmt, ##arg)

#define perr(fmt, arg...)    printf("dev err: " fmt, ##arg)


/* 通信端口 */
#define SIN_PORT 6669

#define CHAECK_CODE  0xAA55

typedef struct 
{
	unsigned char bit_left:1;
	unsigned char bit_right:1;
	unsigned char bit_middle:1;
	unsigned char bit3:1;
	unsigned char bit4:1;
	unsigned char bit5:1;
	unsigned char bit6:1;
	unsigned char bit7:1;
	int rel_x;
	int rel_y;
	int rel_wheel;
}mouse_report_data;

typedef struct
{
	unsigned int code;

	mouse_report_data data;
}udata_t;

static int vms_fd = 0x00;

static udata_t report_data;

static int sock;
static struct sockaddr_in peeraddr;
static socklen_t peerlen = sizeof(peeraddr);

int main(int argc, char *argv[])
{
    int n;

    vms_fd = open("/dev/vms0", O_RDWR);

    if(vms_fd < 0)
    {
        perr("open /dev/vms0 err!!\n");

        return -1;
    }

    memset(&report_data, 0, sizeof(udata_t));

    report_data.code = CHAECK_CODE;

    //int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perr("socket error!!\n");

		return -1;
	}

	struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SIN_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    peerlen = sizeof(peeraddr);

    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		perr("bind error!!\n");

		return -1;
	}

    while(1)
    {
        memset(&report_data.data, 0, sizeof(mouse_report_data));
		
        n = recvfrom(sock, &report_data.data, sizeof(mouse_report_data), 0, (struct sockaddr *)&peeraddr, &peerlen);
		
        if (n == -1)
        {
            if (errno == EINTR)
			{
				continue;
			}

            perr("recvfrom error!!\n");

            return -1;
        }
        else if(n > 0)
        {	
            pdebug("1 L: %d, R: %d, X: %d, Y: %d\n", report_data.data.bit_left, report_data.data.bit_right, \
                                                   report_data.data.rel_x, report_data.data.rel_y);

            report_data.data.rel_x = ntohl(report_data.data.rel_x);
            report_data.data.rel_y = ntohl(report_data.data.rel_y);

            pdebug("2 L: %d, R: %d, X: %d, Y: %d\n", report_data.data.bit_left, report_data.data.bit_right, \
                                       report_data.data.rel_x, report_data.data.rel_y);
                    
            write(vms_fd, (const unsigned char*)&report_data, sizeof(udata_t));

            pinfo("write /dev/vms0 !!!\n");
        }
    }

    close(sock);

    close(vms_fd);

    return 0;
}


