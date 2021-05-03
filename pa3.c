#include "LinkedList.h"

int running = 1;
LinkedList* keys;

// the argument we will pass to the connection-handler threads
struct connection {
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int fd;
};

int server(char *port);
void *echo(void *arg);

// check if a string has ONLY positive numbers -- checks for "12abcd" and "1abcd2" cases
int isLegalAtoiInput(char *arr) {
    for (char *ptr = arr; *ptr; ++ptr)
        if (!(*ptr <= 57 && *ptr >= 48)) // character is not btwn '0' and '9'
            return false;
    return true;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        fflush(stderr);
		return EXIT_FAILURE;
	}
    if(isLegalAtoiInput(argv[1]) == false || atoi(argv[1]) < 5000 || atoi(argv[1]) > 65536) {
        fprintf(stderr, "%s\n", "Illegal port number inputted");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    keys = initLL();
    (void) server(argv[1]);
    destroyLL(keys);
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
    hint.ai_flags = AI_PASSIVE; // setting AI_PASSIVE means that we want to create a listening socket


    error = getaddrinfo(NULL, port, &hint, &info_list);
    if (error != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        fflush(stderr);
        return -1;
    }

    // attempt to create socket
    for (info = info_list; info != NULL; info = info->ai_next) {
        sfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        
        // if we couldn't create the socket, try the next method
        if (sfd == -1) {
            continue;
        }

        if ((bind(sfd, info->ai_addr, info->ai_addrlen) == 0) &&
            (listen(sfd, SOMAXCONN) == 0)) {
            break;
        }

        close(sfd);
    }

    if (info == NULL) {
        // we reached the end of result without successfuly binding a socket
        fprintf(stderr, "Could not bind\n");
        fflush(stderr);
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
        if(con==NULL){
            perror("Malloc Failed in server initiziling con");
            exit(EXIT_FAILURE);
        }
        con->addr_len = sizeof(struct sockaddr_storage);
        
        // wait for an incoming connection
        con->fd = accept(sfd, (struct sockaddr *) &con->addr, &con->addr_len);
        
        // if we got back -1, it means something went wrong
        if (con->fd == -1) {
            perror("accept");
            continue;
        }
        
        // temporarily block SIGINT (child will inherit mask)
        error = pthread_sigmask(SIG_BLOCK, &mask, NULL);
        if (error != 0) {
        	fprintf(stderr, "sigmask: %s\n", strerror(error));
            fflush(stderr);
        	abort();
        }

		// spin off a worker thread to handle the remote connection
        error = pthread_create(&tid, NULL, echo, con);

		// if we couldn't spin off the thread, clean up and wait for another connection
        if (error != 0) {
            fprintf(stderr, "Unable to create thread: %d\n", error);
            fflush(stderr);
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
    free(con);
	pthread_detach(pthread_self());
	pthread_exit(NULL);

    // never reach here
    return 0;
}


void *echo(void *arg) {
    
   // char* load = NULL;
    int BUFSIZE = 8;
    char* buf = malloc(sizeof(char) * BUFSIZE);
    if(buf == NULL){
        perror("Malloc Failed in echo() initializing buf");
        exit(EXIT_FAILURE);
    }
    char currChar; 
    char host[100], port[10];
    struct connection *c = (struct connection *) arg;
    int error, nread;

	// find out the name and port of the remote host
    error = getnameinfo((struct sockaddr *) &c->addr, c->addr_len, host, 100, port, 10, NI_NUMERICSERV);
    	
    if (error != 0) {
        fprintf(stderr, "getnameinfo: %s", gai_strerror(error));
        fflush(stderr);
        close(c->fd);
        return NULL;
    }

    printf("[%s:%s] connection\n", host, port);
    fflush(stdout);

    int getRequest = 0;
    int GET = 0;
    int SET = 0;
    int DEL = 0;
    long int getLoad = 0;
    int getKey = 0;

    char* value = NULL;
    char* key = NULL;
    char* load = NULL;

    while ((nread = read(c->fd, &currChar, 1)) > 0) {
        //if (DEBUG) printf("GET: %d\tSET: %d: DEL: %d\tInput: %s", GET, SET, DEL, buf);
        int bytesRead = 0;
        while(currChar != '\n') {
            if(bytesRead == BUFSIZE) {
                buf = realloc(buf, sizeof(char) * BUFSIZE * 2);
                if(buf == NULL) {
                    write(c->fd, "ERR\nSRV\n", 8);
                    perror("Realloc Failed in echo() with buf");
                    abort();
                }
                BUFSIZE *= 2;
            }
            buf[bytesRead++] = currChar;
            nread = read(c->fd, &currChar, 1);
        }

        if((bytesRead+2) >= BUFSIZE) {
            buf = realloc(buf, sizeof(char) * BUFSIZE * 2);
            if(buf == NULL) {
                write(c->fd, "ERR\nSRV\n", 8);
                perror("Realloc Failed in echo() with buf");
                abort();
            }
            BUFSIZE *= 2;
            buf[bytesRead++] = '\n';
            buf[bytesRead] = '\0';
        }
        else {
            buf[bytesRead++] = '\n';
            buf[bytesRead] = '\0';
        }

        if(getRequest == 0) {
            if(bytesRead != 4) {
                write(c->fd, "ERR\nBAD\n", 8);
                break;
            }
            char key1[] = "GET";
            char key2[] = "SET";
            char key3[] = "DEL";
            if(strncmp(buf, key1,3) == 0) {
                GET = 1;
                getRequest = 1;
                if(DEBUG) printf("Input size (in bytes): %ld\tRequest made: %s", strlen(buf),buf);
            }
            else if(strncmp(buf, key2,3) == 0) {
                SET = 1;
                getRequest = 1;
                if(DEBUG) printf("Input size (in bytes): %ld\tRequest made: %s", strlen(buf),buf);
            }
            else if(strncmp(buf, key3,3) == 0) {
                DEL = 1;
                getRequest = 1;
                if(DEBUG) printf("Input size (in bytes): %ld\tRequest made: %s", strlen(buf),buf);
            }
            else {
                write(c->fd, "ERR\nBAD\n", 8);
                break;
            }
        }
        else if(getLoad == 0) {
            for(int i = 0; i < bytesRead-1; i++) {
                if(!isdigit(buf[i])) {
                    write(c->fd, "ERR\nBAD\n", 8);
                    getRequest = 0;
                    getLoad = -1;
                    break;
                }
            }
            if(getLoad == -1) {
                getLoad = 0;
                break;
            }
            if(DEBUG) printf("Input size (in bytes): %ld\tLoad: %ld\n", strlen(buf), atol(buf));
            if(atol(buf) <= 0) {
                getRequest = 0;
                write(c->fd, "ERR\nBAD\n", 8);
                break;
            }
            else getLoad = atol(buf);
        }
        else if(getKey == 0) {
            key = calloc(bytesRead, sizeof(char));
            if(key == NULL){
                write(c->fd, "ERR\nSRV\n", 8);
                perror("Calloc Failed in echo() with key");
                free(buf);
                exit(EXIT_FAILURE);
            }
            memcpy(key, buf, bytesRead-1);
            if (DEBUG) printf("Input size (in bytes): %ld\tKey: %s\n", strlen(buf), key);
            if(GET == 1) {
                if(getLoad != bytesRead) {
                    getRequest = 0;
                    getLoad = 0;
                    write(c->fd, "ERR\nLEN\n", 8);
                    break;
                }
                else {
                    value = getValueAtKey(keys, key);
                    if(value == NULL) write(c->fd, "KNF\n", 4);
                    else {

                        long int len = snprintf( NULL, 0, "%ld", strlen(value)+1) + 1;
                        load = malloc(sizeof(char) * len);
                        if (load == NULL){
                            write(c->fd, "ERR\nSRV\n", 8);
                            perror("Malloc Failure in echo() for load");
                            free(key);
                            free(buf);
                            exit(EXIT_FAILURE);
                        }
                        write(c->fd, "OKG\n", 4);
                        snprintf(load, len, "%ld", strlen(value)+1);
                        write(c->fd, load, len);
                        write(c->fd, "\n", 1);

                        write(c->fd, value, strlen(value));
                        write(c->fd, "\n", 1);

                        free(load);
                        load = NULL;
                    }
                    free(key);
                    key = NULL;
                    value = NULL;
                    GET = 0;
                }
            }
            else if(SET == 1) {
                read(c->fd, &currChar, 1);
                int bytesRead = 0;
                while(currChar != '\n') {
                    if(bytesRead == BUFSIZE) {
                        buf = realloc(buf, sizeof(char) * BUFSIZE * 2);
                        if(buf == NULL) {
                            write(c->fd, "ERR\nSRV\n", 8);
                            perror("Realloc Failed in echo() with buf");
                            free(key);
                            abort();
                        }
                        BUFSIZE *= 2;
                    }
                    buf[bytesRead++] = currChar;
                    nread = read(c->fd, &currChar, 1);
                }
                if((bytesRead+2) >= BUFSIZE) {
                    buf = realloc(buf, sizeof(char) * BUFSIZE * 2);
                    if(buf == NULL) {
                        write(c->fd, "ERR\nSRV\n", 8);
                        perror("Realloc Failed in echo() with buf");
                        free(key);
                        abort();
                    }
                    BUFSIZE *= 2;
                    buf[bytesRead++] = '\n';
                    buf[bytesRead] = '\0';
                }
                else {
                    buf[bytesRead++] = '\n';
                    buf[bytesRead] = '\0';
                }

                
                value = calloc(bytesRead , sizeof(char));
                if(value == NULL){
                    write(c->fd, "ERR\nSRV\n", 8);
                    perror("Realloc Failed in echo() with value");
                    free(buf);
                    free(key);
                    exit(EXIT_FAILURE);
                }
                memcpy(value, buf, bytesRead-1);
                if(DEBUG) printf("Input size (in bytes): %ld\tValue: %s\n", strlen(buf), value);
                int keyBytes = strlen(key) + 1;
                if((bytesRead + keyBytes) != getLoad) {
                    getRequest = 0;
                    getLoad = 0;
                    write(c->fd, "ERR\nLEN\n", 8);
                    break;
                }
                else {
                    char* temp = getValueAtKey(keys, key);
                    if(temp == NULL) addNode(keys, key, value);
                    else {
                        changeNodeValue(keys, key, value);
                        free(key);
                    }
                    write(c->fd, "OKS\n", 4);
                    SET = 0;
                    key = NULL;
                    value = NULL;
                }
            }
            else if(DEL == 1) {
                if(getLoad != bytesRead) {
                    getRequest = 0;
                    getLoad = 0;
                    write(c->fd, "ERR\nLEN\n", 8);
                    break;
                }
                else {
                    value = getValueAtKey(keys, key);
                    if(value == NULL) {
                        free(key);
                        key = NULL;
                        write(c->fd, "KNF\n", 4);
                    }
                    else {
                        value = deleteKey(keys, key);
                        write(c->fd, "OKD\n", 4);

                        long int len = snprintf( NULL, 0, "%ld", strlen(value)+1) + 1;
                        load = malloc(sizeof(char) * len);
                        if (load == NULL){
                            write(c->fd, "ERR\nSRV\n", 8);
                            perror("Malloc Failure in echo() for load");
                            free(buf);
                            free(key);
                            exit(EXIT_FAILURE);
                        }
                        snprintf(load, len, "%ld", strlen(value)+1);
                        write(c->fd, load, len);
                        write(c->fd, "\n", 1);

                        write(c->fd, value, strlen(value));
                        write(c->fd, "\n", 1);
                        
                        free(key);
                        free(value);
                        free(load);
                        load = NULL;
                        value = NULL;
                        key = NULL;
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
    fflush(stdout);
    if(key != NULL) free(key);
    if(value != NULL) free(value);
    free(buf);
    close(c->fd);
    free(c);
    return NULL;
}
