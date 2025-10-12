#include <arpa/inet.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define SERVER_IP "192.168.249.9"
#define PORT 9

// Transport format (one line)
// A-ALL=aaaaaaaaaaaa;A_A=aaaaaaaaaaaa;A_B=aaaaaaaaaaaa;A_C=aaaaaaaaaaaa;A_D=aaaaaaaaaaaa
// T_ALL=aaaaaaaaaaaa;T_A=aaaaaaaaaaaa;T_B=aaaaaaaaaaaa,T_C=aaaaaaaaaaaa,T_D=aaaaaaaaaaaa\n

static int64_t a_all = 0;
static int64_t a_a = 0;
static int64_t a_b = 0;
static int64_t a_c = 0;
static int64_t a_d = 0;

static int64_t t_all = 0;
static int64_t t_a = 0;
static int64_t t_b = 0;
static int64_t t_c = 0;
static int64_t t_d = 0;

static void format_and_replace(int64_t input, char *output, size_t size) {
  snprintf(output, size, "%012" PRId64, input);

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
  srand(time(NULL));

  a_a += rand() % 1000;
  a_b += rand() % 100;
  a_c += rand() % 10;
  a_d += rand() % 5;
  a_all = a_a + a_b + a_c + a_d;

  t_a += rand() % 10000;
  t_b += rand() % 1000;
  t_c += rand() % 100;
  t_d += rand() % 10;
  t_all = t_a + t_b + t_c + t_d;

  char a_all_str[32];
  char a_a_str[32];
  char a_b_str[32];
  char a_c_str[32];
  char a_d_str[32];

  char t_all_str[32];
  char t_a_str[32];
  char t_b_str[32];
  char t_c_str[32];
  char t_d_str[32];

  format_and_replace(a_all, a_all_str, sizeof(a_all_str));
  format_and_replace(a_a, a_a_str, sizeof(a_a_str));
  format_and_replace(a_b, a_b_str, sizeof(a_b_str));
  format_and_replace(a_c, a_c_str, sizeof(a_c_str));
  format_and_replace(a_d, a_d_str, sizeof(a_d_str));

  format_and_replace(t_all, t_all_str, sizeof(t_all_str));
  format_and_replace(t_a, t_a_str, sizeof(t_a_str));
  format_and_replace(t_b, t_b_str, sizeof(t_b_str));
  format_and_replace(t_c, t_c_str, sizeof(t_c_str));
  format_and_replace(t_d, t_d_str, sizeof(t_d_str));

  snprintf(buffer, buffer_size,
           "A_ALL=%s;A_A=%s;A_B=%s;A_C=%s;A_D=%s;"
           "T_ALL=%s;T_A=%s;T_B=%s;T_C=%s;T_D=%s\n",
           a_all_str, a_a_str, a_b_str, a_c_str, a_d_str, t_all_str, t_a_str,
           t_b_str, t_c_str, t_d_str);
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
