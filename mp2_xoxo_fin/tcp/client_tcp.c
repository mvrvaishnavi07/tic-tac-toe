#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8084

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[4096] = {0};
    
    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to the server.\n");

    // Game loop
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        
        // Receive game state and message from server
        read(sock, buffer, sizeof(buffer));
        printf("%s", buffer);

        // Enter move if prompted
        if (strstr(buffer, "Enter row and column")) {
            printf("vaishuu\n");
            int move_row, move_col;
            printf("Enter your move (row and column): ");
            scanf("%d %d", &move_row, &move_col);
            send(sock, &move_row, sizeof(move_row), 0);
            send(sock, &move_col, sizeof(move_col), 0);
        }

        // Check for rematch prompt
        if (strstr(buffer, "Do you want to play again?")) {
            int response;
            printf("1 for Yes, 0 for No: ");
            scanf("%d", &response);
            send(sock, &response, sizeof(response), 0);
            // break;  // Exit the loop as the game is ending
            // Check if opponent does not want to continue
            memset(buffer, 0, sizeof(buffer));
            read(sock, buffer, sizeof(buffer));
            printf("%s", buffer);
            
            // printf("hloo\n");
            if (strstr(buffer, "Your opponent did not wish to play again.")) {
                printf("The game is ending as your opponent does not want to play again.\n");
                break; // Exit the loop, and close the socket
            }
            else if(strstr(buffer, "You did not wish to play again.")){
                printf("The game is ending as you does not want to play again.\n");
                break; // Exit the loop, and close the socket
            }
            else if(strstr(buffer, "Both player disagreed.\n")){
                printf("hi1\n");
                printf("The game is ending as you both dont want to play again.\n");
                break; // Exit the loop, and close the socket
            }
            // else if(strstr(buffer, "Both players disagreed.\n")){
            //     printf("hi2\n");
            //     printf("The game is ending as you both dont want to play again.\n");
            //     break; // Exit the loop, and close the socket
            // }
            if(response == 0){
                printf("response 0\n");
                break;
            }
        }
    }
    memset(buffer, 0, sizeof(buffer));

    // Close socket
    close(sock);
    return 0;
}
