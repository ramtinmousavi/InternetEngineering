

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
    // result of mathematic operation
    int res;
    // number of readed data
    int num;
    // counters for parsing input data
    int counter;
    int counter2;
    // operands of mathematic expr.
    int operand1;
    int operand2;

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
        // child process to do mathematic requests concurrently
		if (!fork()){
            // avoid zombey process!
			close (fd_server);

            while (1){
                // clear buffer
                memset(buf, 0, 2047);
                // recv data from client
                num = recv(fd_client, buf, sizeof (buf), 0);

                // it means if there is some data
                if (num > 0) {
                    // null terminal for input data
                    buf [num] = '\0';

                    // use expression variable to parse mathematic expr.
                    char expression [num];
                    strcpy (expression, buf);
                    puts (expression);

                    // parsing mathematic expr.
                    counter = 0;
                    char number1 [num];
                    while (expression[counter] != ' '){
                        number1[counter] = expression[counter];
                        counter ++;
                    }
                    number1 [counter] = '\0';
                    counter ++;

                    char operator = expression[counter];

                    counter += 2;
                    counter2 = 0;
                    char number2 [num];
                    while (expression[counter] != '\0'){
                        number2[counter2] = expression[counter];
                        counter ++;
                        counter2 ++;
                    }
                    number2 [counter2] = '\0';

                    operand1 = atoi (number1);
                    operand2 = atoi (number2);

                    // do calculations based on operator
                    if (operator = '+'){
                        res = operand1 + operand2;
                    }else if (operator = '-'){
                        res = operand1 - operand2;
                    }else if (operator = '*'){
                        res = operand1 * operand2;
                    }else if (operator = '/'){
                        res = operand1 / operand2;
                    }else{
                        res = 0;
                    }

                    // fill buffer with integer result of calculations
                    sprintf(buf,"%d\n",res);
                    // send result to client
                    send (fd_client, buf, strlen(buf), 0);
                }
                // if user wants to close connection
                else if (num == 0){
                    perror ("Connection Close\n");
                    break;
                }else{
                    perror("Something wrong in receiving data\n");
                    break;
                }

        }
			// the following line (sleep(1)) exists just to demonstrate that the server is not parallel
			// you may remove it after completing phase 1
			sleep(1);
		    printf("Closing (sleep for 1 sec) ... \n");
		    close(fd_client);
            // exit child process
			exit (0);
		}
        // close accept socket of client in parrent process & loop again to accept new clients!
		close (fd_client);



    }
    return 0;
}
