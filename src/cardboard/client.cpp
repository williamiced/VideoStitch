#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define SERV_PORT 9527
#define MAXDATA   1024
#define MAXNAME 1024

using namespace cv;

int main(int argc, char **argv){
        int fd;     /* fd into transport provider */
        int i;     /* loops through user name */
        int length;    /* length of message */
        int size;    /* the length of servaddr */
        int fdesc;    /* file description */
        int ndata;    /* the number of file data */
        char data[MAXDATA]; /* read data form file */
        char data1[MAXDATA];  /*server response a string */
        char buf[BUFSIZ];     /* holds message from server */
        struct hostent *hp;   /* holds IP address of server */
        struct sockaddr_in myaddr;   /* address that client uses */
        struct sockaddr_in servaddr; /* the server's full addr */

        /*
         * Check for proper usage.
         */
        if (argc < 3) {
                fprintf (stderr, "Usage: %s host_name(IP address) file_name\n", argv[0]);
                exit(2);
        }
        /*
         *  Get a socket into UDP
         */
        if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror ("socket failed!");
                exit(1);
        }
        /*
         * Bind to an arbitrary return address.
         */
        bzero((char *)&myaddr, sizeof(myaddr));
        myaddr.sin_family = AF_INET;
        myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        myaddr.sin_port = htons(0);

        if (bind(fd, (struct sockaddr *)&myaddr,
                                sizeof(myaddr)) <0) {
                perror("bind failed!");
                exit(1);
        }
        /*
         * Fill in the server's UDP/IP address
         */

        bzero((char *)&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(SERV_PORT);
        hp = gethostbyname(argv[1]);

        if (hp == 0) {
                fprintf(stderr, "could not obtain address of %s\n", argv[2]);
                return (-1);
        }
        bcopy(hp->h_addr_list[0], (caddr_t)&servaddr.sin_addr,
                        hp->h_length);

        /*
        fdesc = open(argv[2], O_RDONLY);
        if (fdesc == -1) {
                perror("open file error!");
                exit (1);
        }
        ndata = read (fdesc, data, MAXDATA);
        if (ndata < 0) {
                perror("read file error !");
                exit (1);
        }
        data[ndata + 1] = '\0';
        */

        /*
        Mat img = imread("tmp.png");
        img = img.reshape(0, 1);
        int imgSize = img.total() * img.elemSize();

        size = sizeof(servaddr);
        if (sendto(fd, img.data, imgSize, 0, (struct sockaddr*)&servaddr, size) == -1) {
                perror("write to server error !");
                exit(1);
        }
        printf ("Successfully send image\n");
        */

        size = sizeof(servaddr);
        strcpy(data, "VideoStitcher");

        if (sendto(fd, data, strlen(data) + 1, 0, (struct sockaddr*)&servaddr, size) == -1) {
                perror("write to server error !");
                exit(1);
        }
        printf ("Successfully send image\n");
        
        while (true) {
            if (recvfrom(fd, data1, MAXDATA, 0, (struct sockaddr*)&servaddr, (socklen_t*)&size) < 0) {
                    perror ("read from server error !");
                    exit (1);
            }
            printf("Get image data \n");
        }
        /* 印出 server 回應 **/
        //printf("%s\n", data1);

}