#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8084

char board[3][3];  // Tic-Tac-Toe board
int current_player; // 1 for Player 1, 2 for Player 2

void initialize_board() {
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            board[i][j] = ' ';
        }     
    }
        
}

void print_board(int sock) {
    char message[4096];
    snprintf(message, sizeof(message), "Current Board:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            snprintf(message + strlen(message), sizeof(message) - strlen(message), "%c ", board[i][j]);
        }
        snprintf(message + strlen(message), sizeof(message) - strlen(message), "\n");
    }
    send(sock, message, strlen(message), 0);
}

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

void play_game(int new_socket1, int new_socket2) {
    current_player = 1; // Player 1 starts
    int move_row, move_col;
    int winner;

    while (1) {
        print_board(new_socket1);
        print_board(new_socket2);
        if (current_player == 1) {
            printf("vaish\n");
            send(new_socket1, "Your turn (Enter row and column): ", 35, 0);
            recv(new_socket1, &move_row, sizeof(move_row), 0);
            recv(new_socket1, &move_col, sizeof(move_col), 0);
        } else {
            send(new_socket2, "Your turn (Enter row and column): ", 35, 0);
            recv(new_socket2, &move_row, sizeof(move_row), 0);
            recv(new_socket2, &move_col, sizeof(move_col), 0);
        }

        // Validate the move
        if (move_row < 0 || move_row >= 3 || move_col < 0 || move_col >= 3 || board[move_row][move_col] != ' ') {
            if (current_player == 1) {
                send(new_socket1, "Invalid move! Column is out of bounds. Try again.\n", 50, 0);
            } else {
                send(new_socket2, "Invalid move! Column is out of bounds. Try again.\n", 50, 0);
            }
            continue;
        }

        // Place the move
        if (current_player == 1) {
            board[move_row][move_col] = 'X';
            // printf("Player 1 placed 'X' at (%d, %d)\n", move_row, move_col);
        } else {
            board[move_row][move_col] = 'O';
            // printf("Player 2 placed 'O' at (%d, %d)\n", move_row, move_col);
        }

        winner = check_winner();

        // Check for winner or draw
        if (winner) {
            print_board(new_socket1);
            print_board(new_socket2);
            char message[4096];
            snprintf(message, sizeof(message), "Player %d Wins!\n", winner);
            send(new_socket1, message, strlen(message), 0);
            send(new_socket2, message, strlen(message), 0);
            break;
        }

        if (is_draw()) {
            print_board(new_socket1);
            print_board(new_socket2);
            send(new_socket1, "It's a Draw!\n", 14, 0);
            send(new_socket2, "It's a Draw!\n", 14, 0);
            // printf("hii\n");
            break;
        }

        // Switch player
        if (current_player == 1) {
            current_player = 2;
            // printf("Switching to Player 2's turn.\n");
        } else {
            current_player = 1;
            // printf("Switching to Player 1's turn.\n");
        }

    }

    // // Ask for rematch
    // int response1, response2;
    // printf("vaish\n");
    // send(new_socket1, "Do you want to play again? (1 for Yes, 0 for No): ", 52, 0);
    // recv(new_socket1, &response1, sizeof(response1), 0);
    // send(new_socket2, "Do you want to play again? (1 for Yes, 0 for No): ", 52, 0);
    // recv(new_socket2, &response2, sizeof(response2), 0);
    // printf("res 1 %d\n",response1);
    // printf("res 2 %d\n",response2);

    // Ask for rematch
    int response1, response2;
    send(new_socket1, "Do you want to play again? (1 for Yes, 0 for No): ", 52, 0);
    recv(new_socket1, &response1, sizeof(response1), 0);
    send(new_socket2, "Do you want to play again? (1 for Yes, 0 for No): ", 52, 0);
    recv(new_socket2, &response2, sizeof(response2), 0);
    printf("res 1 %d\n", response1);
    printf("res 2 %d\n", response2);

    // Check responses
    if (response1 == 1 && response2 == 1) {
        // Reset and play again
        printf("rematch\n");
        initialize_board();
        play_game(new_socket1, new_socket2);
    } else if (response1 == 0 && response2 == 0) {
        // Both players do not want to play again
        // printf("hi\n");
        send(new_socket1, "Both player disagreed.\n", 22, 0);
        send(new_socket2, "Both players disagreed.\n", 23, 0);
        close(new_socket1);
        close(new_socket2);
        printf("Game ended. Connections closed.\n");
    } else {
        // One player wants to continue, the other does not
        if (response1 == 1 && response2 == 0) {
            send(new_socket1, "Your opponent did not wish to play again.\n", 43, 0);
            send(new_socket2, "You did not wish to play again.\n", 34, 0);
        } else {
            send(new_socket2, "Your opponent did not wish to play again.\n", 43, 0);
            send(new_socket1, "You did not wish to play again.\n", 34, 0);
        }
        // Close both connections
        close(new_socket1);
        close(new_socket2);
        printf("Game ended. Connections closed.\n");
    }

}

int main() {
    int server_fd, new_socket1, new_socket2;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Attach socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 2) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for players to connect...\n");

    // Accept two clients
    new_socket1 = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    printf("Player 1 connected.\n");
    new_socket2 = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    printf("Player 2 connected.\n");

    initialize_board();
    play_game(new_socket1, new_socket2);

    // Close the sockets
    close(new_socket1);
    close(new_socket2);
    close(server_fd);

    return 0;
}
