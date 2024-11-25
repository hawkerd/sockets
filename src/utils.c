#include "../include/utils.h"
#include "../include/server.h"
#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <stdint.h>

//TODO: Declare a global variable to hold the file descriptor for the server socket
int server_socket;

//TODO: Declare a global variable to hold the mutex lock for the server socket
pthread_mutex_t server_socket_mtx = PTHREAD_MUTEX_INITIALIZER;

//TODO: Declare a gloabl socket address struct to hold the address of the server
struct sockaddr_in server_address;


// SERVER FUNCTIONS

/**********************************************
 * init
   - port is the number of the port you want the server to be
     started on
   - initializes the connection acception/handling system
   - if init encounters any errors, it will call exit().
************************************************/
void init(int port) {
  int socket_fd;
  //struct sockaddr_in address;
   
  // Create a socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    perror("System call socket() failed");
    exit(EXIT_FAILURE);
  }
  
  // Set socket options
  int enable = 1;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    perror("System call setsockopt() failed");
    exit(EXIT_FAILURE);
  }

  // Initialize the address struct
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(port);

  // Bind the socket
  if (bind(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
      perror("System call bind() failed");
      exit(EXIT_FAILURE);
  }

  // Mark the socket as a pasive socket
  if (listen(socket_fd, SOMAXCONN) < 0) {
      perror("System call listen() failed");
      exit(EXIT_FAILURE);
  }
  
  // We save the file descriptor to a global variable so that we can use it in accept_connection().
  server_socket = socket_fd;

  printf("UTILS.O: Server Started on Port %d\n",port);
  fflush(stdout);
}


/**********************************************
 * accept_connection - takes no parameters
   - returns a file descriptor for further request processing.
   - if the return value is negative, the thread calling
     accept_connection must should ignore request.
***********************************************/
int accept_connection(void) {
  struct sockaddr_in new_addr;
  socklen_t addr_len = sizeof(new_addr);
  int newsock;
  
  // Acquire the mutex lock
  if (pthread_mutex_lock(&server_socket_mtx) < 0) {
    perror("System call lock() failed");
    return -1;
  }

  // Accept a new connection on the passive socket
  newsock = accept(server_socket, (struct sockaddr*)&new_addr, &addr_len);
  if (newsock < 0) {
    pthread_mutex_unlock(&server_socket_mtx);
    perror("System call accept() failed");
    return -1;
  }
  
  // Release the mutex lock
  if (pthread_mutex_unlock(&server_socket_mtx) < 0) {
    perror("System call unlock() failed");
    return -1;
  }

  return newsock;
}


/**********************************************
 * send_file_to_client
   - socket is the file descriptor for the socket
   - buffer is the file data you want to send
   - size is the size of the file you want to send
   - returns 0 on success, -1 on failure 
************************************************/
int send_file_to_client(int socket, char * buffer, int size) {
  // Send the file size packet
  packet_t packet;
  packet.size = size;

  if (send(socket, &packet, sizeof(packet), 0) < 0) {
    perror("System call send() failed");
    return -1;
  }

  // Send the file data
  int bytes_sent = 0;
  while (bytes_sent < size) {
    int sent_bytes = send(socket, buffer + bytes_sent, (size_t)size - bytes_sent, 0);
    if (sent_bytes < 0) {
      perror("System call send() failed");
      return -1;
    }
    bytes_sent += sent_bytes;
  }

  return 0;
}


/**********************************************
 * get_request_server
   - fd is the file descriptor for the socket
   - filelength is a pointer to a size_t variable that will be set to the length of the file
   - returns a pointer to the file data
************************************************/
char * get_request_server(int fd, size_t *filelength)
{
  // Receive the size packet
  packet_t packet;
  if (recv(fd, &packet, sizeof(packet), 0) < 0) {
    perror("System call recv() failed");
    return NULL;
  }
  
  // Get the size of the image
  *filelength = packet.size;
  if (packet.size == 0) {
    printf("Received empty image");
    return NULL;
  }

  // Allocate memory
  char* buffer = malloc(*filelength);
  if (buffer == NULL) {
    perror("System call malloc() failed");
    return NULL;
  }

  // Receive data
  size_t bytes_received = 0;
  while (bytes_received < packet.size) {
    int received_bytes = recv(fd, buffer + bytes_received, packet.size - bytes_received, 0);
    if (received_bytes < 0) {
      perror("System call perror() failed");
      free(buffer);
      return NULL;
    }
    bytes_received += received_bytes;
  }

  return buffer;
}


/*
################################################
##############Client Functions##################
################################################
*/

/**********************************************
 * setup_connection
   - port is the number of the port you want the client to connect to
   - initializes the connection to the server
   - if setup_connection encounters any errors, it will call exit().
************************************************/
int setup_connection(int port)
{
    //TODO: create a sockaddr_in struct to hold the address of the server   

    //TODO: create a socket and save the file descriptor to sockfd
   
    //TODO: assign IP, PORT to the sockaddr_in struct

    //TODO: connect to the server
   
    //TODO: return the file descriptor for the socket
}


/**********************************************
 * send_file_to_server
   - socket is the file descriptor for the socket
   - file is the file pointer to the file you want to send
   - size is the size of the file you want to send
   - returns 0 on success, -1 on failure
************************************************/
int send_file_to_server(int socket, FILE *file, int size) 
{
    //TODO: send the file size packet
   

    //TODO: send the file data
   

    // TODO: return 0 on success, -1 on failure
   
}

/**********************************************
 * receive_file_from_server
   - socket is the file descriptor for the socket
   - filename is the name of the file you want to save
   - returns 0 on success, -1 on failure
************************************************/
int receive_file_from_server(int socket, const char *filename) 
{
    //TODO: create a buffer to hold the file data
    

    //TODO: open the file for writing binary data
   
    
   //TODO: create a packet_t to hold the packet data
    
   //TODO: receive the response packet
  

    //TODO: get the size of the image from the packet
   
   

   //TODO: recieve the file data and write it to the file
    
    //TODO: return 0 on success, -1 on failure
}

