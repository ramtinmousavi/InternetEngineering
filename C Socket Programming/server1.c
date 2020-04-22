//
// Created by mammalofski on 10/12/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

// webpage to show
char webpage[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n\r\n"
        "<!DOCTYPE html>\r\n"
        "<html><head><title>Our Sample Page</title><link rel=\"icon\" href=\"testicon.ico\">\r\n"
        "<style>body {background-color: #FFFF00}</style></head>\r\n"
        "<body><center><h1>Hello World!</h1><br>\r\n"
        "<img src=\"testpic.jpg\"><img src=\"tstpic2.jpg\"></center></body></html>\r\n";

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_len = sizeof(client_addr);
    //client & server sockets
    int fd_server, fd_client;
    // buffer to read and write data
    char buf[2048];
    int fdimg;
    // this variable is used to avoid port binding err
    int on = 1;

    // server socket
    fd_server = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_server < 0) {
		    perror("socket");
		    exit(1);
		}

    // avoid address in use err.
	if (setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1){
		perror ("setsockopt");
		exit (1);
	}

    // assign server addr.
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    // binding server socket
    if (bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(fd_server);
        exit(1);
    }

    // listen socket with capacity of 10
    if (listen(fd_server, 10) == -1) {
        perror("listen");
        close(fd_server);
        exit(1);
    }

    while (1) {
        // create accept socket for every new connection
        fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);
        if (fd_client == -1) {
            perror("Connection Failed! Can't Conneect to Client .... \n");
            continue;
        }
        printf("Accepted the Client Connection ..... \n");

		if (!fork()){ // child process for new client

            // avoid zombey process!
			close (fd_server);
            // clear buffer
			memset(buf, 0, 2047);
            // read data from buff
		    read(fd_client, buf, 2047);
		    printf("%s\n", buf);

            // response according to req.
		    if (!strncmp(buf, "GET /testpic.jpg", 16)) {
		        fdimg = open("testpic.jpg", O_RDONLY);
		        sendfile(fd_client, fdimg, NULL, 60000);
		        close(fdimg);
		    } else if (!strncmp(buf, "GET /tstpic2.jpg", 16)) {
		        fdimg = open("tstpic2.jpg", O_RDONLY);
		        sendfile(fd_client, fdimg, NULL, 120000);
		        close(fdimg);
		    } else if (!strncmp(buf, "GET /testicon.ico", 16)) {
 		        fdimg = open("testicon.ico", O_RDONLY);
 		        sendfile(fd_client, fdimg, NULL, 200000);
 		        close(fdimg);
 		    } else {
		        write(fd_client, webpage, sizeof(webpage) - 1);
		    }
			// the following line (sleep(1)) exists just to demonstrate that the server is not parallel
			// you may remove it after completing phase 1

		    printf("Closing (Sleep for 3 seconds) ... \n");
		    sleep(3);
		    close(fd_client);
			exit (0);
		}

		close (fd_client);



    }
    return 0;
}
