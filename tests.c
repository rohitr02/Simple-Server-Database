#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <ctype.h>
#include "LinkedList.h"
#define BACKLOG 5

int running = 1;

// the argument we will pass to the connection-handler threads
struct connection {
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int fd;
};

int server(char *port);
void *echo(void *arg);

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Usage: %s [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

    (void) server(argv[1]);
    return EXIT_SUCCESS;
}

void handler(int signal)
{
	running = 0;
}


int server(char *port)
{
    struct addrinfo hint, *info_list, *info;
    struct connection *con;
    int error, sfd;
    pthread_t tid;

    // initialize hints
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;
    	// setting AI_PASSIVE means that we want to create a listening socket

    // get socket and address info for listening port
    // - for a listening socket, give NULL as the host name (because the socket is on
    //   the local host)
    error = getaddrinfo(NULL, port, &hint, &info_list);
    if (error != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    // attempt to create socket
    for (info = info_list; info != NULL; info = info->ai_next) {
        sfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        
        // if we couldn't create the socket, try the next method
        if (sfd == -1) {
            continue;
        }

        // if we were able to create the socket, try to set it up for
        // incoming connections;
        // 
        // note that this requires two steps:
        // - bind associates the socket with the specified port on the local host
        // - listen sets up a queue for incoming connections and allows us to use accept
        if ((bind(sfd, info->ai_addr, info->ai_addrlen) == 0) &&
            (listen(sfd, BACKLOG) == 0)) {
            break;
        }

        // unable to set it up, so try the next method
        close(sfd);
    }

    if (info == NULL) {
        // we reached the end of result without successfuly binding a socket
        fprintf(stderr, "Could not bind\n");
        return -1;
    }

    freeaddrinfo(info_list);

	struct sigaction act;
	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);
	
	sigset_t mask;
	
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);


    // at this point sfd is bound and listening
    printf("Waiting for connection...\n");
	while (running) {
    	// create argument struct for child thread
		con = malloc(sizeof(struct connection));
        con->addr_len = sizeof(struct sockaddr_storage);
        	// addr_len is a read/write parameter to accept
        	// we set the initial value, saying how much space is available
        	// after the call to accept, this field will contain the actual address length
        
        // wait for an incoming connection
        con->fd = accept(sfd, (struct sockaddr *) &con->addr, &con->addr_len);
        	// we provide
        	// sfd - the listening socket
        	// &con->addr - a location to write the address of the remote host
        	// &con->addr_len - a location to write the length of the address
        	//
        	// accept will block until a remote host tries to connect
        	// it returns a new socket that can be used to communicate with the remote
        	// host, and writes the address of the remote hist into the provided location
        
        // if we got back -1, it means something went wrong
        if (con->fd == -1) {
            perror("accept");
            continue;
        }
        
        // temporarily block SIGINT (child will inherit mask)
        error = pthread_sigmask(SIG_BLOCK, &mask, NULL);
        if (error != 0) {
        	fprintf(stderr, "sigmask: %s\n", strerror(error));
        	abort();
        }

		// spin off a worker thread to handle the remote connection
        error = pthread_create(&tid, NULL, echo, con);

		// if we couldn't spin off the thread, clean up and wait for another connection
        if (error != 0) {
            fprintf(stderr, "Unable to create thread: %d\n", error);
            close(con->fd);
            free(con);
            continue;
        }

		// otherwise, detach the thread and wait for the next connection request
        pthread_detach(tid);

        // unblock SIGINT
        error = pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
        if (error != 0) {
        	fprintf(stderr, "sigmask: %s\n", strerror(error));
        	abort();
        }

    }

	puts("No longer listening.");
	pthread_detach(pthread_self());
	pthread_exit(NULL);

    // never reach here
    return 0;
}


void *echo(void *arg) {
    int BUFSIZE = 8;
    char* buf = malloc(sizeof(char) * BUFSIZE);
    char host[100], port[10];
    struct connection *c = (struct connection *) arg;
    int error, nread;

	// find out the name and port of the remote host
    error = getnameinfo((struct sockaddr *) &c->addr, c->addr_len, host, 100, port, 10, NI_NUMERICSERV);
    	// we provide:
    	// the address and its length
    	// a buffer to write the host name, and its length
    	// a buffer to write the port (as a string), and its length
    	// flags, in this case saying that we want the port as a number, not a service name
    if (error != 0) {
        fprintf(stderr, "getnameinfo: %s", gai_strerror(error));
        close(c->fd);
        return NULL;
    }

    printf("[%s:%s] connection\n", host, port);

    int getRequest = 0;
    int GET = 0;
    int SET = 0;
    int DEL = 0;
    int getLoad = 0;
    int getKey = 0;
    LinkedList* keys = initLL();
    while ((nread = read(c->fd, buf, BUFSIZE)) > 0) {
        buf[nread] = '\0';

        if(getRequest == 0) {
            if(nread != 4) {
                printf("BAD\n");
                break;
            }
            char key1[] = "GET";
            char key2[] = "SET";
            char key3[] = "DEL";
            if(strncmp(buf, key1,3) == 0) {
                GET = 1;
                getRequest = 1;
                if(DEBUG) printf("Request: %s", buf);
            }
            else if(strncmp(buf, key2,3) == 0) {
                SET = 1;
                getRequest = 1;
                if(DEBUG) printf("Request: %s", buf);
            }
            else if(strncmp(buf, key3,3) == 0) {
                DEL = 1;
                getRequest = 1;
                if(DEBUG) printf("Request: %s", buf);
            }
            else {
                printf("BAD\n");
                break;
            }
        }
        else if(getLoad == 0) {
            if(DEBUG) printf("Load: %s", buf);
            if(atoi(buf) <= 0) {
                getRequest = 0;
                printf("BAD\n");
                break;
            }
            else getLoad = atoi(buf);
        }
        else if(getKey == 0) {
            char* key = malloc(sizeof(char) * nread-1);
            memcpy(key, buf, nread-1);
            if (DEBUG) printf("Key: %s\tBytes: %d\n", key,nread);
            if(GET == 1) {
                if(getLoad != nread) {
                    getRequest = 0;
                    getLoad = 0;
                    printf("LEN\n"); 
                    break;
                }
                else {
                    char* value = getValueAtKey(keys, key);
                    if(value == NULL) printf("KNF\n");
                    else printf("OKG\n%ld\n%s\n", strlen(value)+1, value);
                    GET = 0;
                }
            }
            if(SET == 1) {
                nread = read(c->fd, buf, BUFSIZE);
                char* value = malloc(sizeof(char) * nread);
                memcpy(value, buf, nread-1);
                int keyBytes = strlen(key) + 1;
                if((nread + keyBytes) != getLoad) {
                    getRequest = 0;
                    getLoad = 0;
                    printf("LEN\n"); 
                    break;
                }
                else {

                    char* temp = getValueAtKey(keys, key);
                    if(DEBUG) printf("Value: %s\n", value);
                    if(temp == NULL) addNode(keys, key, value);
                    else changeNodeValue(keys, key, value);
                    printf("OKS\n");
                    SET = 0;
                }
            }
            if(DEL == 1) {
                if(getLoad != nread) {
                    getRequest = 0;
                    getLoad = 0;
                    printf("LEN\n"); 
                    break;
                }
                else {
                    if (DEBUG) printf("Delete key\n");

                    char* value = getValueAtKey(keys, key);
                    if(value == NULL) printf("KNF\n");
                    else {
                        value = deleteKey(keys, key);
                        printf("OKD\n%ld\n%s\n", strlen(value)+1, value);
                        free(value);
                    }
                    DEL = 0;
                }
            }
            getRequest = 0;
            getLoad = 0;
            if(DEBUG) {
                printLL(keys);
                putchar('\n');
            }
        }
    }

    printf("[%s:%s] disconnected\n", host, port);
    free(buf);
    destroyLL(keys);
    close(c->fd);
    free(c);
    return NULL;
}