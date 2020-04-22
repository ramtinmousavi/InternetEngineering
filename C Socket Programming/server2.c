

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

char webpage[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n\r\n"
        "<!DOCTYPE html>\r\n"
        "<html><head><title>Our Sample Page</title><link rel=\"icon\" href=\"testicon.ico\">\r\n"
        "<style>body {background-color: #FFFF00}</style></head>\r\n"
        "<body><center><h1>Hello World!</h1><br>\r\n"
        "<img src=\"testpic.jpg\"><img src=\"tstpic2.jpg\"></center></body></html>\r\n";




int main(int argc, char *argv[])
{
    // create master set and a copy of it to use in select syscall
    fd_set master;
    fd_set read_fds;


    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_len = sizeof(client_addr);

    // max number of fds
    int max_fd;

    int listen_socket;
    int new_accpet_socket;
    char buf[2048];
    // number of readed bytes
    int nbytes;
    int on = 1;
    int fdimg;
    // use in loops
    int i;
    int j;

    // clear both sets
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    // server socket
    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_socket < 0){
        perror("socket");
        exit(1);
    }

    // avoid address in use err.
    if(setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1){
        perror("setsockopt");
        exit(1);
    }

    // assign server addr.
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    memset(&(server_addr.sin_zero), '\0', 8);


    // binding server socket
    if(bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        perror("bind");
        close (listen_socket);
        exit(1);
    }

    // listen socket with capacity of 10
    if(listen(listen_socket, 10) == -1){
         perror("listen");
         close (listen_socket);
         exit(1);
    }

    // add server socket to set
    FD_SET(listen_socket, &master);
    // at the beginning, max fd is listen sock
    max_fd = listen_socket;


    while (1){
        // copy of master set
        read_fds = master;

        // blocking call untill one socket has action
        // in first iretation of the loop, listening socket is the only socket found in read_fds
        if (select(max_fd+1, &read_fds, NULL, NULL, NULL) == -1){
            perror("select");
            exit(1);
        }
        printf("select() is OK...\n");


        for(i = 0; i <= max_fd; i++){
            // if one socket has action while monitoring (select)
            if(FD_ISSET(i, &read_fds)){
                // i is founded socket
                if(i == listen_socket){ // if that socket is listen sock

                    // accept it and make new socket
                    if ((new_accpet_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &sin_len)) == -1){
                        perror("Connection Failed! Can't Conneect to Client .... \n");
                    }
                    else{ // successful socket
                        printf("Accepted the Client Connection ..... \n");
                        // add this socket to master set to be monitored in next iter.
                        FD_SET(new_accpet_socket, &master);
                        if(new_accpet_socket > max_fd){
                            // modify max fd
                            max_fd = new_accpet_socket;
                        }
                    }
                }
                else{ //data from client

                    memset(buf, 0, 2047);
                    if((nbytes = read(i, buf, 2047)) <= 0){

                        if(nbytes == 0) {// con close
                            printf("Connection Close....\n");
                        }
                        else{
                        perror("read");
                        }
                        // close and clear sock from master set
                        close(i);
                        FD_CLR(i, &master);

                        // modify max if needed
                        if (i == max_fd){
                            for (j = 0; j < max_fd ; j++){
                                if (FD_ISSET(j, &master)){
                                    max_fd = j;
                                }
                            }
                        }


                    }

                    else{
                        // do same thing as problem 1 !!!
                        printf("Server reading msg from socket #%d : %s\n",i, buf );

                        if (!strncmp(buf, "GET /testicon.ico", 16)) {
            		        fdimg = open("testicon.ico", O_RDONLY);
            		        sendfile(i, fdimg, NULL, 200000);
            		        close(fdimg);

            		    } else if (!strncmp(buf, "GET /testpic.jpg", 16)) {
            		        fdimg = open("testpic.jpg", O_RDONLY);
            		        sendfile(i, fdimg, NULL, 60000);
            		        close(fdimg);

            		    } else if (!strncmp(buf, "GET /tstpic2.jpg", 16)) {
            		        fdimg = open("tstpic2.jpg", O_RDONLY);
            		        sendfile(i, fdimg, NULL, 120000);
            		        close(fdimg);

            		    } else {
            		        write(i, webpage, sizeof(webpage) - 1);
            		    }

                        close(i);
                        FD_CLR(i, &master);


                    }
                    sleep (1);

                }
            }
        }
    }
    return 0;
}
