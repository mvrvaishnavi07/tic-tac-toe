#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define MAX_BUFFER 1024

char board[3][3];
int current_player = 1;

struct client_info {
    struct sockaddr_in address;
    socklen_t addr_len;
};

struct client_info players[2];

void initialize_board() {
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            board[i][j] = ' ';
        }     
    }
        
}

void send_to_player(int sock, int player, const char* message) {
    sendto(sock, message, strlen(message), 0, 
           (struct sockaddr*)&players[player-1].address, 
           players[player-1].addr_len);
}

void print_board(int sock) {
    char message[MAX_BUFFER];
    snprintf(message, sizeof(message), "Current Board:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            snprintf(message + strlen(message), sizeof(message) - strlen(message), 
                     "%c ", board[i][j]);
        }
        snprintf(message + strlen(message), sizeof(message) - strlen(message), "\n");
    }
    send_to_player(sock, 1, message);
    send_to_player(sock, 2, message);
}

// ... [check_winner and is_draw functions remain the same] ...

int check_winner() {
    for (int i = 0; i < 3; i++) {
        // Check rows for a winner
        if (board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
            if (board[i][0] == 'X') {
                return 1;
            } else {
                return 2;
            }
        }

        // Check columns for a winner
        if (board[0][i] != ' ' && board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
            if (board[0][i] == 'X') {
                return 1;
            } else {
                return 2;
            }
        }
    }

    // Check the main diagonal for a winner
    if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
        if (board[0][0] == 'X') {
            return 1;
        } else {
            return 2;
        }
    }

    // Check the anti-diagonal for a winner
    if (board[0][2] != ' ' && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
        if (board[0][2] == 'X') {
            return 1;
        } else {
            return 2;
        }
    }

    // No winner found
    return 0;
}

int is_draw() {
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            if (board[i][j] == ' ') return 0;
        }     
    }
        
    return 1;
}

void play_game(int sock) {
    char buffer[MAX_BUFFER];
    int move_row, move_col;
    int winner;

    while (1) {
        print_board(sock);
        
        snprintf(buffer, sizeof(buffer), "Player %d's turn (Enter row and column): ", current_player);
        send_to_player(sock, current_player, buffer);

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int received = recvfrom(sock, buffer, MAX_BUFFER, 0, 
                                (struct sockaddr*)&client_addr, &client_len);
        
        if (received > 0) {
            sscanf(buffer, "%d %d", &move_row, &move_col);
            
            if (move_row < 0 || move_row >= 3 || move_col < 0 || move_col >= 3 || 
                board[move_row][move_col] != ' ') {
                send_to_player(sock, current_player, "Invalid move! Try again.\n");
                continue;
            }

            board[move_row][move_col] = (current_player == 1) ? 'X' : 'O';
            winner = check_winner();

            if (winner || is_draw()) {
                print_board(sock);
                if (winner) {
                    snprintf(buffer, sizeof(buffer), "Player %d Wins!\n", winner);
                } else {
                    snprintf(buffer, sizeof(buffer), "It's a Draw!\n");
                }
                send_to_player(sock, 1, buffer);
                send_to_player(sock, 2, buffer);
                break;
            }

            if (current_player == 1) {
                current_player = 2;
                // printf("Switching to Player 2's turn.\n");
            } else {
                current_player = 1;
                // printf("Switching to Player 1's turn.\n");
            }
        }
    }

    // Rematch logic
    for (int i = 0; i < 2; i++) {
        send_to_player(sock, i+1, "Do you want to play again? (1 for Yes, 0 for No): ");
    }

    int responses[2] = {0, 0};
    for (int i = 0; i < 2; i++) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        recvfrom(sock, &responses[i], sizeof(int), 0, 
                 (struct sockaddr*)&client_addr, &client_len);
    }

    if (responses[0] == 1 && responses[1] == 1) {
        initialize_board();
        play_game(sock);
    } else {
        const char* message = (responses[0] == responses[1]) ? 
            "Both players agreed to end the game.\n" : 
            "One player wanted to continue, but the other did not. Ending game.\n";
        send_to_player(sock, 1, message);
        send_to_player(sock, 2, message);
    }
}

int main() {
    int server_fd;
    struct sockaddr_in address;

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for players to connect...\n");

    // Wait for two players to send initial messages
    for (int i = 0; i < 2; i++) {
        char buffer[MAX_BUFFER];
        players[i].addr_len = sizeof(players[i].address);
        recvfrom(server_fd, buffer, MAX_BUFFER, 0, 
                 (struct sockaddr*)&players[i].address, &players[i].addr_len);
        printf("Player %d connected.\n", i+1);
        send_to_player(server_fd, i+1, "Connected to the game. Waiting for other player...\n");
    }

    send_to_player(server_fd, 1, "You are Player 1 (X). Game is starting.\n");
    send_to_player(server_fd, 2, "You are Player 2 (O). Game is starting.\n");

    initialize_board();
    play_game(server_fd);

    close(server_fd);
    return 0;
}