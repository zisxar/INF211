#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h> /* wait */
#include <sys/types.h> /* sockets*/
#include <sys/socket.h> /* sockets*/
#include <sys/wait.h> /* sockets */
#include <netinet/in.h> /* Internet sockets*/
#include <unistd.h> /* read, write, close, fork, pipe */
#include <netdb.h> /* gethostbyaddr */
#include <signal.h> /* signal */
#include <string.h>

/* Enter the string with the possible port and the position of the '\n'
   Returns -1 in case there isn't a port and the port otherwise */
int extract_port(char* buf, int pos) {
    char ret_val[7] = "000000";
    char* compstr;
    int i, j;

    pos = pos - 8;
    if (pos <= 0)
        return -1;

    compstr = buf + pos;
    if (strcmp(compstr, "meta_end\n") == 0) {
        j = sizeof(ret_val) - 2;
        for (i=(pos-1); i>=0; i--){
            ret_val[j] = buf[i];
            j = j - 1;
        }
        while (j>=0) {
            ret_val[j] = '0';
            j = j - 1;
        }
        return atoi(ret_val);
    }
    return -1;            
}

void child_server(int new_str_sock, in_addr_t addr) {
    char s_buf[1], d_buf[512];
    int i = 0;
    char port_found = 0;

    /***** datagram variables *****/
    int  port_dgr = 0, sock_dgr;
    char buf_dgr[512];
    struct hostent* rem;
    struct sockaddr_in server_dgr, client_dgr; // server -> server_dgr, client -> client_dgr
    unsigned int server_dgr_len = sizeof(server_dgr); // serverlen -> server_dgr_len
    struct sockaddr* server_dgr_ptr = (struct sockaddr*) &server_dgr; // serverptr -> server_dgr_ptr
    struct sockaddr* client_dgr_ptr = (struct sockaddr*) &client_dgr; // clientptr -> client_dgr_ptr
    /*****/

    /* Aquire the UDP port */
    while (read(new_str_sock, s_buf, 1) > 0) { /* Receive 1 char */
        putchar(s_buf[0]); /* Print received char */
        d_buf[i] = s_buf[0];
        if (s_buf[0] == '\n') {
            if (port_dgr = extract_port(d_buf, i) > 0)
                break;
        }
        i = i+1;
    }

    /* Create UDP socket */
    if ((sock_dgr = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    /* Setup server’s IP address and port */
    server_dgr.sin_family = AF_INET;
    server_dgr.sin_addr.s_addr = addr;
    server_dgr.sin_port = htons(port_dgr);
    /* Setup my address */
    client_dgr.sin_family = AF_INET; /* Internet domain */
    client_dgr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any address */
    client_dgr.sin_port = htons(0); /* Autoselect port */
    /* Bind my socket to my address */
    if (bind(sock_dgr, client_dgr_ptr, sizeof(client_dgr)) < 0) {
        perror("bind");
        exit(1);
    }

    i = 0;
    while (read(new_str_sock, s_buf, 1) > 0) { /* Receive 1 char */
        d_buf[i] = s_buf[0];
        putchar(s_buf[0]); /* Print received char */
        if (i >= 512) {
            d_buf[strlen(d_buf) - 1] = '\0'; /* Remove '\n' */
            /* Send message */
            if (sendto(sock_dgr, d_buf, strlen(d_buf)+1, 0, server_dgr_ptr, server_dgr_len) < 0) {
                perror("sendto");
                exit(1);
            }
            
            i = 0;            
        }
        i = i+1;
    }
    if (i < 512) {
        d_buf[strlen(d_buf) - 1] = '\0'; /* Remove '\n' */
        /* Send message */
        if (sendto(sock_dgr, d_buf, strlen(d_buf)+1, 0, server_dgr_ptr, server_dgr_len) < 0) {
            perror("sendto");
            exit(1);
        }
    }

    printf("Closing connection.\n");
    close(new_str_sock); /* Close socket */
    close(sock_dgr); /* Close UDP socket */
}

/* Wait for all dead child processes */
void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void perror_exit(char* message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void main(int argc, char* argv[]) {

    /* Various */
    int num_children, i;
    /* Signals */
    static struct sigaction act;
    int status;

    /* Pipes */
    int p[2];

    /* TCP */
    int port_str, sock_str, new_str_sock; // port -> port_str
    char s_buf[512];
    struct sockaddr_in server, client;
    socklen_t client_len; // clientlen -> client_len
    struct sockaddr* server_ptr = (struct sockaddr*) &server ; // serverptr -> server_ptr
    struct sockaddr* client_ptr = (struct sockaddr*) &client ; // clientptr -> client_ptr
    struct hostent* rem;

    if (argc != 3) {
        printf("Please give TCP port number and number of clients\n");
        exit(1);
    }

    /* Create a pipe used for the communication between the parent and the children*/
    if (pipe(p) == -1) {
        perror("pipe call");
        exit(1);
    }

    /* Reap dead children asynchronously */
    signal(SIGCHLD, sigchld_handler);

    act.sa_handler = SIG_IGN; // the handler is set to IGNORE
    sigfillset(&(act.sa_mask));
    sigaction(SIGPIPE, &act, NULL); // Ignore the signal SIGPIPE

    /* Create stream socket */
    if ((sock_str = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("Error creating stream socket");

    port_str = atoi(argv[1]);

    /* Setup IP address and port */
    server.sin_family = AF_INET;/* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port_str); /* The given port */

    /* Bind stream socket to address */
    if (bind(sock_str, server_ptr, sizeof(server)) < 0)
        perror_exit("Error binding the TCP socket");
    
    num_children = atoi(argv[2]);
    /* Listen for connections */
    if (listen(sock_str, num_children) < 0)
        perror_exit("Error listening for connections");

    while (1) {
        client_len = sizeof(client);
        /* Accept connection */
        if ((new_str_sock = accept(sock_str, client_ptr, &client_len)) < 0)
            perror_exit("accept");
        /* Find client’s address */
        if ((rem = gethostbyaddr((char*) &client.sin_addr.s_addr, sizeof(client.sin_addr.s_addr), client.sin_family)) == NULL) {
            herror("gethostbyaddr");
            exit(1);
        }
        printf("Accepted connection from %s\n", rem->h_name);
            
        switch (fork()) { /* Create child for serving client */
        case -1: /* Error */
            perror("fork");
            break ;
        case 0: /* Child process */
            close(p[1]);
            //rsize = read(p[0], inbuf, MSGSIZE);
            close(sock_str);
            child_server(new_str_sock, client.sin_addr.s_addr);
            exit(0);
        default:
            close(p[0]);
            //write(p[1], msg1, MSGSIZE);
        }
        close(new_str_sock); /* parent closes socket to client */
    }

    close(sock_str); /* Close TCP socket */

    /* Wait until all children have exit */
    while (wait(&status) != -1) {
        sleep(1);
    }

    act.sa_handler = SIG_DFL; // reestablish the DEFAULT behavior
}