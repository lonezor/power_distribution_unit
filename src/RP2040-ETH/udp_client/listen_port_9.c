#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9
#define BUFFER_SIZE 1024

int main() {
  int sockfd;
  char buffer[BUFFER_SIZE];
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len = sizeof(client_addr);

  // 1. Create a UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Clear structure memory
  memset(&server_addr, 0, sizeof(server_addr));
  memset(&client_addr, 0, sizeof(client_addr));

  // Configure server address information
  server_addr.sin_family = AF_INET; // IPv4 protocol
  server_addr.sin_addr.s_addr =
      INADDR_ANY;                     // Accept packets from any IP address
  server_addr.sin_port = htons(PORT); // Listen on the specified port

  // 2. Bind the socket to the port
  if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("bind failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  printf("UDP listener started on port %d...\n", PORT);

  // 3. Receive and print packets in a continuous loop
  while (1) {
    int n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, 0,
                     (struct sockaddr *)&client_addr, &addr_len);

    // Check for read errors
    if (n < 0) {
      perror("recvfrom failed");
      continue;
    }

    // Null-terminate the received data to treat it as a string
    buffer[n] = '\0';

    // Print the received packet to stdout
    printf("Received packet: %s", buffer);
  }

  // 4. Close the socket (this part of the code is unreachable due to the
  // infinite loop)
  close(sockfd);

  return 0;
}
