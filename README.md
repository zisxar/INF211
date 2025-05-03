# Course Assignments: Learning Bash Script, Python, and Sockets

## Description

This is a series of three assignments implemented for the requirements of the university course "Software Development Tools and System Programming INF211" in 2019-2020.  
The assignments were carried out by a two-person team, Lina Tsapanou and Zisis Charokopos.  
The description files of the assignments are not uploaded.

## Technologies Used

- Languages: Bash, Python, C.
- Tools: Linux

## First Assignment

Files:

```text
INF211/
├── assignment_1/
│   ├── regr
│   └── results
├── assignment_2/
├── assignment_3/
└── README.md
```

For the first assignment, two Bash programs were developed.  
The first program takes a list of files as input and calculates the linear regression parameters: a, b, c, and the error.  
The second program takes the results of matches between countries as input and calculates the ranking of each country, categorizing them accordingly.

## Second Assignment

Files:

```text
INF211/
├── assignment_1/
├── assignment_2/
│   └── computerSales.py
├── assignment_3/
└── README.md
```

For the second assignment, a Python program was developed.  
The script takes as input files representing receipts, does a validity check, and prints various statistics that were requested for the assignment implementation.

## Third Assignment

Files:

```text
INF211/
├── assignment_1/
├── assignment_2/
├── assignment_3/
│   ├── Makefile
│   ├── remoteClient.c
│   └── remoteServer.c
└── README.md
```

For the third assignment, the requirements were to implement a server that receives symbols from a client, and returns them back. The connection from the client to the server should use TCP sockets, while the connection for sending the symbols back should utilize UDP sockets.

To achieve this, two C files were developed: `remoteServer.c` and `remoteClient.c`. In both programs, the parent process is designed to wait until all child processes have completed their execution, which is handled using the `wait()` function.

In the `remoteServer` program, appropriate variable declarations are made, a parameter count check is performed, and the `SIGPIPE` signal is ignored using `sigaction`. A TCP socket is created with the necessary parameters configured.

To establish communication between the parent and child processes, a pipe is created. The write end of the pipe is kept open in the parent process and closed in the child process, while the reverse applies to the reading end. This setup is essential for effectively managing the child processes from the parent.

When a new connection is accepted, a child process is created using the `fork()` function. This new child process then executes the `child_server()` function. Within this function, a UDP socket is created. The UDP port is sent by the client through the TCP socket. To send back the symbols, the server operates as a client while the client functions as a server for the UDP connection.

## Contributors

- [Lina Tsapanou](https://github.com/LinaTsapanou)
- [Zisis Charokopos](https://github.com/zisxar)
