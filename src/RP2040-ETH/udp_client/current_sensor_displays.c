#include <arpa/inet.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define MICROSECONDS_PER_SECOND 1000000
#define NANOSECONDS_PER_MICROSECOND 1000

#define SERVER_IP "192.168.249.4"
#define PORT 9

// Extended format:
//  - 0-9: visible numbers
//  - a: segments off for number
//  - b-d: decimal point (most significant point first) as 5th symbol
//  - e: clock indication
//  - f: disabled
//
// It is easier to manage with fixed position. Index d0-d5 is used, physically
// top-down
//
// Example: 43 as aa43f
// Example: 1234 as 1234f
// Example: 3.141 as 3141b (.b.c.d)
// Example: 12:45 as 1245e
//
// Transport format:
// D_0=aaaaf;D_1=aaaaf;D_2=aaaaf;D_3=aaaaf;D_4=aaaaf;D_5=aaaaf\n

static int d0 = 0; // range: 0-9999
static int d1 = 0; // range: 0-9999
static int d2 = 0; // range: 0-9999
static int d3 = 0; // range: 0-9999
static int d4 = 0; // range: 0-9999
static int d5 = 0; // range: 0-9999

static char d0_suffix = 0xf; // range: 0xa - 0xf
static char d1_suffix = 0xf; // range: 0xa - 0xf
static char d2_suffix = 0xf; // range: 0xa - 0xf
static char d3_suffix = 0xf; // range: 0xa - 0xf
static char d4_suffix = 0xf; // range: 0xa - 0xf
static char d5_suffix = 0xf; // range: 0xa - 0xf

static void format_and_replace(int64_t input, char *output, size_t size) {
  memset(output, 0, size); // this allows future expansion
  snprintf(output, size, "%04" PRId64, input);

  for (int i = 0; i < strlen(output); i++) {
    if (output[i] == '0') {
      output[i] = 'a';
    } else {
      // Stop once we encounter the first non-zero digit
      break;
    }
  }
}

void generate_next_state(char *buffer, int buffer_size) {

  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
    perror("clock_gettime failed");
    exit(1);
  }

  unsigned int seed = (unsigned int)(ts.tv_sec * MICROSECONDS_PER_SECOND +
                                     ts.tv_nsec / NANOSECONDS_PER_MICROSECOND);

  srand(seed);

  d0 = (rand() % 10000) % 10000;
  d1 = (rand() % 1000) % 10000;
  d2 = (rand() % 100) % 10000;
  d3 = (rand() % 50) % 10000;
  d4 = (rand() % 25) % 10000;
  d5 = (rand() % 5) % 10000;

  char d0_str[32];
  char d1_str[32];
  char d2_str[32];
  char d3_str[32];
  char d4_str[32];
  char d5_str[32];

  format_and_replace(d0, d0_str, sizeof(d0_str));
  format_and_replace(d1, d1_str, sizeof(d1_str));
  format_and_replace(d2, d2_str, sizeof(d2_str));
  format_and_replace(d3, d3_str, sizeof(d3_str));
  format_and_replace(d4, d4_str, sizeof(d4_str));
  format_and_replace(d5, d5_str, sizeof(d5_str));

  if (rand() % 10000 < 10) {
    d0_str[4] = 'b';
  } else {
    d0_str[4] = 'f';
  }

  if (rand() % 1000 < 10) {
    d1_str[4] = 'c';
  } else {
    d1_str[4] = 'f';
  }

  if (rand() % 100 < 10) {
    d2_str[4] = 'd';
  } else {
    d2_str[4] = 'f';
  }

  if (rand() % 50 < 10) {
    d3_str[4] = 'e';
  } else {
    d3_str[4] = 'f';
  }

  if (rand() % 25 < 10) {
    d4_str[4] = 'b';
  } else {
    d4_str[4] = 'f';
  }

  if (rand() % 5 < 10) {
    d5_str[4] = 'c';
  } else {
    d5_str[4] = 'f';
  }

  snprintf(buffer, buffer_size, "D_0=%s;D_1=%s;D_2=%s;D_3=%s;D_4=%s;D_5=%s\n",
           d0_str, d1_str, d2_str, d3_str, d4_str, d5_str);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Usage: send_msg <iterations> <usec_delay>\n");
    return 0;
  }

  int iterations = atoi(argv[1]);
  int usec_delay = atoi(argv[2]);

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

  for (int i = 0; i < iterations; i++) {

    char tx_buffer[1400];
    memset(tx_buffer, 0, sizeof(tx_buffer));

    generate_next_state(tx_buffer, sizeof(tx_buffer));

    printf("%s", tx_buffer);

    ssize_t sent_bytes =
        sendto(sockfd, tx_buffer, strlen(tx_buffer), 0,
               (const struct sockaddr *)&server_addr, sizeof(server_addr));

    usleep(usec_delay);
  }

  close(sockfd);

  return 0;
}
