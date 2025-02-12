#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define MAX_BUFFER 4096

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER] = {0};
    
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Send initial message to server
    sendto(sock, "Connect", 7, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    printf("Sent connection request to server.\n");

    while (1) {
        socklen_t server_len = sizeof(server_addr);
        int received = recvfrom(sock, buffer, MAX_BUFFER, 0, 
                                (struct sockaddr*)&server_addr, &server_len);
        
        if (received > 0) {
            printf("%s", buffer);

            if (strstr(buffer, "Enter row and column")) {
                int move_row, move_col;
                printf("Enter your move (row and column): ");
                scanf("%d %d", &move_row, &move_col);
                char move[10];
                snprintf(move, sizeof(move), "%d %d", move_row, move_col);
                sendto(sock, move, strlen(move), 0, 
                       (struct sockaddr*)&server_addr, sizeof(server_addr));
            }

            if (strstr(buffer, "Do you want to play again?")) {
                int response;
                printf("1 for Yes, 0 for No: ");
                scanf("%d", &response);
                sendto(sock, &response, sizeof(response), 0, 
                       (struct sockaddr*)&server_addr, sizeof(server_addr));
            }

            if (strstr(buffer, "Ending game") || strstr(buffer, "agreed to end the game")) {
                printf("Game ended. Closing connection.\n");
                break;
            }
        }

        memset(buffer, 0, sizeof(buffer));
    }

    close(sock);
    return 0;
}