#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h> /* wait */
#include <sys/types.h> /* sockets*/
#include <sys/socket.h> /* sockets*/
#include <netinet/in.h> /* Internet sockets*/
#include <unistd.h> /* read , write , close, fork, pipe */
#include <netdb.h> /* gethostbyname */
#include <arpa/inet.h> /* inet_ntoa */
#include <string.h>

char* name_from_address(struct in_addr addr) {
    struct hostent* rem;
    int asize = sizeof(addr.s_addr);
    if ((rem = gethostbyaddr(&addr.s_addr, asize, AF_INET)))
        return rem->h_name; /* reverse lookup success */
    return inet_ntoa(addr); /* fallback to a.b.c.d form */
}

void perror_exit(char* message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void main(int argc, char* argv[]) {

    pid_t pid;
    int status;

    int port_str, sock_str, i; // port -> port_str
    char s_buf[512];
    struct sockaddr_in server_str; // server -> server_str
    struct sockaddr* server_str_ptr = (struct sockaddr*) &server_str ; // serverptr -> server_str_ptr
    struct hostent* rem;

    /* datagram variables */
    int port_dgr, sock_dgr, n;
    unsigned int server_dgr_len, client_dgr_len; // serverlen -> server_dgr_len, clientlen -> client_dgr_len
    char d_buf[512], * client_name; // clientname -> client_name
    struct sockaddr_in server_dgr, client_dgr;
    struct sockaddr* server_dgr_ptr = (struct sockaddr*) &server_dgr; // serverptr -> server_dgr_ptr
    struct sockaddr* client_dgr_ptr = (struct sockaddr*) &client_dgr; // clientptr -> client_dgr_ptr

    if (argc != 5) {
        printf("Please give host name, server port number, receive port number and input file with commands\n");
        exit(1);
    }

    /* Create stream socket */
    if ((sock_str = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("Error creating stream socket");

    /* Create datagram socket */
    if ((sock_dgr = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        perror_exit("Error creating datagram socket");

    /* Find server address */
    if ((rem = gethostbyname(argv[1])) == NULL) {
        herror("Error in gethostbyname");
        exit(1);
    }

    port_dgr =  atoi(argv[3]);
    /* Bind datagram socket to address */
    server_dgr.sin_family = AF_INET; /* Internet domain */
    server_dgr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_dgr.sin_port = htons(port_dgr); /* Autoselect a port */
    server_dgr_len = sizeof(server_dgr);
    if (bind(sock_dgr, server_dgr_ptr, server_dgr_len ) < 0)
        perror_exit("bind");
    /* Discover selected port */
    if (getsockname(sock_dgr, server_dgr_ptr, &server_dgr_len) < 0)
        perror_exit("getsockname");
    printf("Socket port : %d\n", ntohs(server_dgr.sin_port));

    port_str = atoi(argv[2]);
    /* Setup server’s IP address and port */
    server_str.sin_family = AF_INET;/* Internet domain */
    memcpy(&server_str.sin_addr, rem->h_addr, rem->h_length);
    server_str.sin_port = htons(port_str); /* Server port */

    /* Initiate TCP connection */
    if (connect(sock_str, server_str_ptr, sizeof(server_str)) < 0)
        perror_exit("Error connecting to the server");

    printf("Connecting to %s port %d\n", argv[1], port_str);

    /////////////////////
    // create metadata //
    /////////////////////
    sprintf(s_buf, "%dmeta_end\n", port_dgr);
    for (i=0; s_buf[i]!='\0'; i++) { /* For every char */
        if (write(sock_str, s_buf+i, 1) < 0) /* Send i-th character */
            perror_exit("Error on write");
    }

    switch (pid = fork()) { /* Create child for sending data */
    case -1: /* Error */
        perror("fork");
    case 0: /* Child process */
        //////////////////////////
        // send stuff using TCP //
        //////////////////////////
        do {
            printf("Child:%d\n", getpid());
            printf("Give input string : ");
            fgets(s_buf, sizeof(s_buf), stdin); /* Read from stdin */
            for (i=0; s_buf[i]!='\0'; i++) { /* For every char */
                if (write(sock_str, s_buf+i, 1) < 0) /* Send i-th character */
                    perror_exit("Error on write");
            }
        } while (strcmp(s_buf, "end\n") != 0); /* Finish on "end" */
        
        exit(0);
    }

    if (pid != 0) {
        switch (pid = fork()) { /* Create child for sending data */
        case -1: /* Error */
            perror("fork");
        case 0: /* Child process */
            /////////////////////////////
            // receive stuff using UDP //
            /////////////////////////////
            while (1) { 
                printf("Child:%d\n", getpid());
                client_dgr_len = sizeof(client_dgr);
                /* Receive message */
                if ((n = recvfrom(sock_dgr, d_buf, sizeof(d_buf), 0, client_dgr_ptr, &client_dgr_len)) < 0)
                    perror("recvfrom");
                printf("n1\n");
                d_buf[sizeof(d_buf)-1] = '\0'; /* Force str termination */
                /* Try to discover client ’s name */
                client_name = name_from_address(client_dgr.sin_addr);
                printf("Received from %s : %s\n", client_name, d_buf);
            }
        }
    }

    /* Wait until all children have exit */
    while (wait(&status) != -1) {
        sleep(1);
    }

    close(sock_str); /* Close TCP socket */
    close(sock_dgr); /* Close UDP socket */

}