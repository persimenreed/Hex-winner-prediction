// Copyright (c) 2024 Ole-Christoffer Granmo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef BOARD_DIM
#define BOARD_DIM 11
#endif

// Neighbor positions
int neighbors[] = {-(BOARD_DIM+2) + 1, -(BOARD_DIM+2), -1, 1, (BOARD_DIM+2), (BOARD_DIM+2) - 1};

// Define the hex game structure
struct hex_game {
    int board[(BOARD_DIM+2)*(BOARD_DIM+2)*2]; // Two bits per cell for player 0 and player 1
    int open_positions[BOARD_DIM*BOARD_DIM];
    int number_of_open_positions;
    int moves[BOARD_DIM*BOARD_DIM]; // Record of moves made
    int move_player[BOARD_DIM*BOARD_DIM]; // Record of which player made the move
    int connected[(BOARD_DIM+2)*(BOARD_DIM+2)*2];
};

// Initialize the game
void hg_init(struct hex_game *hg)
{
    for (int i = 0; i < (BOARD_DIM+2)*(BOARD_DIM+2)*2; ++i) {
        hg->board[i] = 0;
        hg->connected[i] = 0;
    }

    hg->number_of_open_positions = 0;

    for (int i = 0; i < BOARD_DIM+2; ++i) {
        for (int j = 0; j < BOARD_DIM+2; ++j) {
            int pos = i*(BOARD_DIM + 2) + j;
            if (i > 0 && i < BOARD_DIM + 1 && j > 0 && j < BOARD_DIM + 1) {
                hg->open_positions[hg->number_of_open_positions++] = pos;
            }
            if (i == 0) {
                hg->connected[pos * 2] = 1; // Player 0 top edge
            }
            if (j == 0) {
                hg->connected[pos * 2 + 1] = 1; // Player 1 left edge
            }
        }
    }
}

// Function to connect pieces recursively
int hg_connect(struct hex_game *hg, int player, int position)
{
    hg->connected[position*2 + player] = 1;
    if (player == 0 && position / (BOARD_DIM + 2) == BOARD_DIM + 1) {
        return 1;
    }
    if (player == 1 && position % (BOARD_DIM + 2) == BOARD_DIM + 1) {
        return 1;
    }
    for (int i = 0; i < 6; ++i) {
        int neighbor = position + neighbors[i];
        if (neighbor < 0 || neighbor >= (BOARD_DIM+2)*(BOARD_DIM+2)) continue;
        if (hg->board[neighbor*2 + player] && !hg->connected[neighbor*2 + player]) {
            if (hg_connect(hg, player, neighbor)) {
                return 1;
            }
        }
    }
    return 0;
}

// Function to check if a player has won
int hg_winner(struct hex_game *hg, int player, int position)
{
    for (int i = 0; i < 6; ++i) {
        int neighbor = position + neighbors[i];
        if (neighbor < 0 || neighbor >= (BOARD_DIM+2)*(BOARD_DIM+2)) continue;
        if (hg->connected[neighbor*2 + player]) {
            return hg_connect(hg, player, position);
        }
    }
    return 0;
}

// Place a piece randomly for a player
int hg_place_piece_randomly(struct hex_game *hg, int player)
{
    if (hg->number_of_open_positions == 0) return -1;
    int random_empty_position_index = rand() % hg->number_of_open_positions;
    int empty_position = hg->open_positions[random_empty_position_index];
    hg->board[empty_position * 2 + player] = 1;
    int move_index = (BOARD_DIM * BOARD_DIM) - hg->number_of_open_positions;
    hg->moves[move_index] = empty_position;
    hg->move_player[move_index] = player;
    hg->open_positions[random_empty_position_index] = hg->open_positions[hg->number_of_open_positions - 1];
    hg->number_of_open_positions--;
    return empty_position;
}

// Check if the board is full
int hg_full_board(struct hex_game *hg)
{
    return hg->number_of_open_positions == 0;
}

// Function to generate board string with given number of moves removed
void generate_board_string(struct hex_game *hg, int moves_to_remove, char *board_str) {
    // Create a copy of the board
    int board_copy[(BOARD_DIM+2)*(BOARD_DIM+2)*2];
    for (int i = 0; i < (BOARD_DIM+2)*(BOARD_DIM+2)*2; ++i) {
        board_copy[i] = hg->board[i];
    }
    // Remove the specified number of moves
    int moves_played = (BOARD_DIM * BOARD_DIM) - hg->number_of_open_positions;
    int moves_to_keep = moves_played - moves_to_remove;
    if (moves_to_keep < 0) moves_to_keep = 0;
    for (int i = moves_to_keep; i < moves_played; ++i) {
        int pos = hg->moves[i];
        int player = hg->move_player[i];
        board_copy[pos * 2 + player] = 0;
    }
    // Convert the board to a string representation
    int idx = 0;
    for (int i = 1; i <= BOARD_DIM; ++i) {
        for (int j = 1; j <= BOARD_DIM; ++j) {
            int pos = i * (BOARD_DIM + 2) + j;
            if (board_copy[pos * 2] == 1) {
                board_str[idx++] = 'X';
            } else if (board_copy[pos * 2 + 1] == 1) {
                board_str[idx++] = 'O';
            } else {
                board_str[idx++] = ' ';
            }
        }
    }
    board_str[idx] = '\0';
}

int main() {
    srand((unsigned int)time(NULL)); // Seed the random number generator

    struct hex_game hg;
    int winner = -1;

    // Open CSV files for writing game results
    FILE *fp_full = fopen("/Users/persi/VSCode-H24/IKT457/GeneratingBoards/test_finished.csv", "w");
    FILE *fp_minus2 = fopen("/Users/persi/VSCode-H24/IKT457/GeneratingBoards/test_2bf.csv", "w");
    FILE *fp_minus5 = fopen("/Users/persi/VSCode-H24/IKT457/GeneratingBoards/test_5bf.csv", "w");
    if (!fp_full || !fp_minus2 || !fp_minus5) {
        perror("Unable to open one or more files");
        return 1;
    }

    fprintf(fp_full, "board,winner\n");   // CSV header for fully played games
    fprintf(fp_minus2, "board,winner\n"); // CSV header for minus 2 moves games
    fprintf(fp_minus5, "board,winner\n"); // CSV header for minus 5 moves games

    int total_games = 0;
    int max_games = 10000000;

    for (int game = 0; game < max_games; ++game) {
        hg_init(&hg);
        int player = 0;
        winner = -1;

        while (!hg_full_board(&hg)) {
            int position = hg_place_piece_randomly(&hg, player);
            if (position == -1) break;
            if (hg_winner(&hg, player, position)) {
                winner = player;
                break;
            }
            player = 1 - player;
        }

        int moves_played = (BOARD_DIM * BOARD_DIM) - hg.number_of_open_positions;

        if (winner != -1 && hg.number_of_open_positions >= 75) {
            total_games++;

            char board_full[BOARD_DIM * BOARD_DIM + 1];
            char board_minus2[BOARD_DIM * BOARD_DIM + 1];
            char board_minus5[BOARD_DIM * BOARD_DIM + 1];

            generate_board_string(&hg, 0, board_full);    // No moves removed
            generate_board_string(&hg, 2, board_minus2);  // Last 2 moves removed
            generate_board_string(&hg, 5, board_minus5);  // Last 5 moves removed

            // Printing games to csv
            fprintf(fp_full, "\"%s\",%d\n", board_full, winner);
            fprintf(fp_minus2, "\"%s\",%d\n", board_minus2, winner);
            fprintf(fp_minus5, "\"%s\",%d\n", board_minus5, winner);

            // Console vizualisation
            /*
            printf("\nPlayer %d wins!\n\n", winner);
            hg_print(&hg);
            */

            printf("Generated game %d\n", total_games);
        }

        // Defining the amount of games
        if (total_games >= 100000) {
            break;
        }
    }

    fclose(fp_full);
    fclose(fp_minus2);
    fclose(fp_minus5);

    printf("All games generated\n");

    return 0;
}
