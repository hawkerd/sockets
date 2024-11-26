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

int server_socket; // server socket fd
pthread_mutex_t server_socket_mtx = PTHREAD_MUTEX_INITIALIZER; // socket mutex
struct sockaddr_in server_address; // server address

// SERVER FUNCTIONS

/**
 * init(): initializes the connectino acception/handling system by creating a socket and binding it to the specified port.
 * if any system calls fail, calls exit().
 */
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

/**
 * accept_connection(): accepts a connection from the server socket, and creates a new socket to handle it.
 * uses a mutex lock to ensure no race conditions. if any system calls fail, returns -1;
 */
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


/**
 * send_file_to_client(): used to send a file to the client, from the server, using the specified socket.
 * if any system calls fail, returns -1.
 */
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

/**
 * get_request_server(): receives an image from the client, using the given file descriptor. saves the length in
 * the in/out variable filelength. returns a pointer to the array containing the image. if any system calls fail,
 * exits
 */
char * get_request_server(int fd, size_t *filelength) {
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


// CLIENT FUNCTIONS

/**
 * setup_connection(): initializes the connection with the server. It will create a socket and bind it to
 * the specified port argument. If any of the system calls fail, it will terminate.
 */
int setup_connection(int port) { 
    struct sockaddr_in new_addr;

    // Create a socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Construct address
    new_addr.sin_family = AF_INET;
    new_addr.sin_port = htons(port);
    new_addr.sin_addr.s_addr = server_address.sin_addr.s_addr;

    // Connect to the server
    if (connect(fd, (struct sockaddr*)&new_addr, sizeof(new_addr)) < 0) {
        perror("Connection failed");
        close(fd);
        exit(1);
    }

    // Return the file descriptor for the socket
    return fd;
}

/**
 * send_file_to_server(): writes the size of the file to the server through the specified socket. Then, writes the file
 * in chunks. If any system call fails, it will return -1.
 */
int send_file_to_server(int socket, FILE *file, int size) {
  packet_t packet;
  packet.size = size;

  //TODO: send the file size packet
  if (write(socket, &packet, sizeof(packet)) != sizeof(packet)) {
      perror("Failed to send file size");
      return -1;
  }

  //TODO: send the file data
  char buffer[1024];
  int bytes_sent = 0;
  int bytes_read;

  while (bytes_sent < size) {
      bytes_read = fread(buffer, 1, sizeof(buffer), file);
      if (bytes_read < 0) {
          perror("Failed to read from file");
          return -1;
      }

      int bytes_written = write(socket, buffer, bytes_read);
      if (bytes_written < 0) {
          perror("Failed to send file data");
          return -1;
      }

      bytes_sent += bytes_written;
  }

    // TODO: return 0 on success, -1 on failure
   return 0;
}

/**
 * receive_file_from_server(): acts as an intermediary between the socket and the output file. Reads information from the socket
 * and places it into the file. If any system call fails, returns -1.
 */
int receive_file_from_server(int socket, const char *filename) {
  //TODO: create a buffer to hold the file data
  char buffer[1024];

  // Open the file
  FILE* fh = fopen(filename, "wb");
  if(fh == NULL){
    perror("Failed to open file");
    return -1;
  }
  
  // Receive the size packet
  packet_t packet;
  if(recv(socket, &packet, sizeof(packet), 0) < 0){
    perror("Failed to receive");
    return -1;
  }

  // Get the size of the image
  int file_size = packet.size;
  if (file_size <= 0) {
      fprintf(stderr, "Invalid file size received\n");
      return -1;
  } 

  // Receive data and write to file
  int bytes_received = 0;
  while (bytes_received < file_size) {
    int received = recv(socket, buffer, sizeof(buffer), 0);
    if (received <= 0) {
        perror("Failed to receive file data");
        return -1;
    }

    if (fwrite(buffer, 1, received, fh) < (size_t)received) {
          perror("Failed to write to file");
          return -1;
    }

    bytes_received += received;
  }

  return 0;
}

