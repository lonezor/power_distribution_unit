#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "192.168.249.2"
#define PORT 10

int main(int argc, char* argv[]) {
    if (argc < 4) {
       printf("Usage: send_msg <msg> <iterations> <usec_delay>\n");
       return 0;
    } 

    int iterations = atoi(argv[2]);
    int usec_delay = atoi(argv[3]);

    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid server IP address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

   for (int i=0; i< iterations; i++) {

    char tx_buffer[1400];
    memset(tx_buffer, 0, sizeof(tx_buffer));
    snprintf(tx_buffer, sizeof(tx_buffer), "%s #%06d\n", argv[1], i);

    printf("%s", tx_buffer);

    ssize_t sent_bytes = sendto(
        sockfd,
        tx_buffer,
        strlen(tx_buffer),
        0,
        (const struct sockaddr *)&server_addr,
        sizeof(server_addr)
    );

    usleep(usec_delay);
    }

    close(sockfd);

    return 0;
}
