/**
 * Miquela Valdez
 * 900364439
 * Last Edited: May 4, 2026 3:00
 */

#include <notcurses/notcurses.h>
#include <notcurses/direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

/**
 * @brief The size of the board in each mode
 */
typedef enum {
    MODE_9x9_10,    /**< 9x9 board with 10 mines */
    MODE_16x16_40,  /**< 16x16 board with 40 mines */
    MODE_30x16_99,  /**< 30x16 board with 99 mines */
    MODE_CUSTOM,    /**< Custome board size */
} SizeMode;

/**
 * @brief Represents the different types of gameplay.
 */
typedef enum{
    VARIANT_NORMAL,          /**< Normal Minesweeper */
    VARIANT_CHECKERBOARD,   /**< Checkerboard: tile alternates color, bombs on certain tiles count as 2 */
    VARIANT_LIAR,           /**< The Liar: count is off by +-1 if adjacent tiles have mines */
} GameVariant;

/**
 * @brief Configuration settings for the game
 */
typedef struct {
    SizeMode size_mode;     /**< Selected size mode */
    int width;              /**< Width of the board */
    int height;             /**< Height of the board */
    int total_mines;        /**< Total number of mines */
    GameVariant variant;    /**< Selected gameplay variant */
} GameConfig;

/**
 * @brief State of a single cell on the board
 */
 typedef struct {
    bool is_mine;            /**< True if the cell contains a mine */
    bool is_revealed;        /**< True if the cell has been revealed */
    bool is_flagged;         /**< True if the cell has been flagged */
    int adjacent_mines;      /**< Displayed number of adjacent mines */
    int actual_adjacent      /**< Actual number of adjacent mines (for Liar) */
 } Cell;

 /**
  * @brief State of the game board
  */
typedef struct {
    Cell **grid;        /**< 2D array of cells */
    int width;          /**< Width of the board */
    int height;         /**< Height of the board */
    int total_mines;    /**< Total number of mines */
    int flagged_count;  /**< Number of currently flagged cells */
    int revealed_count; /**< Number of currently revealed cells */
    bool game_over;     /**< True if the game is over */
    bool victory;       /**< True if the player won */
    int cursor_x;       /**< X position of the cursor */
    int cursor_y;       /**< Y position of the cursor */
    bool first_click;   /**< True if the first click has not happened yet */
} Board;

/**
 * @brief Sets up the board parameters with chosen mode 
 */
 void apply_size_mode(GameConfig *config) {
    switch (config->size_mode) {
        case MODE_9x9_10:
            config->width = 9;
            config->height = 9;
            config->total_mines = 10
            break;
        case MODE_16x16_40:
            config->width = 16;
            config->height = 16;
            config->total_mines = 40;
            break;
        case MODE_30x16_99:
            config->width = 30;
            config->height = 16;
            config->total_mines = 99;
            break;
        case MODE_CUSTOM:
            //Handled during selection
            break;
    }
 }

 /**
  *@brief Displays the introduction screen and gets user input
  *
  * @param nc The notcurses context
  * @param config The game configuration
  */
void show_intro_screen(struct notcurses* nc, GameConfig *config) {
    struct ncplane* stdn = notcurses_stdplane(nc);
    ncplane_erase(stdn);

    unsigned int dimy, dimx;
    ncplane_dim_yx(stdn, &dimy, &dimx);

    config->size_mode = MODE_9x9_10;
    config->variant = VARIANT_NORMAL;

    int selection = 0; //0: Size Mode, 1: Variant, 2: Start

    struct ncinput ni;
    while (true) {
        ncplane_erase(stdn);

        ncplane_putstr_yx(stdn, 2, (dimx - 11) / 2, "MINESWEEPER");

        ncplane_putstr_yx(stdn, 4, 4, selection == 0 ? ">Size Mode: ");
        const char *size_str = "";
        switch (config->size_mode) {
            case MODE_9x9_10: size_str = "9x9 (10 Mines)"; break;
            case MODE_16x16_40: size_str = "16x16 (40 Mines)"; break;
            case MODE_30x16_99: size_str = "30x16 (99 Mines)"; break;
            case MODE_CUSTOM: size_str = "Custom"; break;
        }
        ncplane_putstr(stdn, size_str);

        if (config->size_mode == MODE_CUSTOM) {
            ncplane_printf_yx(stdn, 5, 8, "Width: %d, Height: %d, Mines: %d", config->width, config->height, config->total_mines);
            ncplane_putstr_yx(stdn, 6, 8, "(Use Q/W for Width, A/S for Height, Z/X for Mines)");
        }

        int y_offset = config->size_mode == MODE_CUSTOM ? 8 : 6;

        ncplane_putstr_yx(stdn, y_offset, 4, selection == 1 ? ">Variant: " : " Variant: ");
        const char *var_str = "";
        switch (config0>variant) {
            case VARIANT_NORMAL: var_str = "Normal"; break;
            case VARIANT_CHECKERBOARD: var_str = "Checkerboard"; break;
            case VARIANT_LIAR: var_str = "The Liar"; break;
        }
        ncplane_putstr(stdn, var_str);

        ncplane_putstr_yx(stdn, y_offset + 2, 4, selection == 2 ? "> Start Game" : " Start Game");
        ncplane_putstr_yx(stdn, dimy - 2, 2, "Use UP/DOWN to navigate, LEFT/RIGHT to change options, Enter to start.");

        notcurses_render(nc);

        uint32_t key = notcurses_get_blockomg(nc, &ni);
        if (key == (uint32_t)-1) continue;

        if (key == NCKEY_UP || key == 'k') {
            selection = (selection + 2) % 3;
        } else if (key == NCKEY_DOWN || key == 'j') {
            selection = (selection + 1) % 3;
        } else if (key == NCKEY_LEFT || key == 'h') {
            if (selection == 0) {
                config->size_mode = (config->size_mode + 3) % 4;
                if (config->size_mode == MODE_CUSTOM) {
                    config->width = 20;
                    config->height = 20;
                    config->total_mines = 50;
                }
            } else if (selection == 1) {
                config->variant = (config->variant + 2) % 3;
            }
        }else if (key == NCKEY_RIGHT || key == '1') {
            if (selection == 0) {
                config->size_mode = (config->size_mode + 1) % 4;
                if (config->size_mode == MODE_CUSTOM) {
                    config->width = 20;
                    config->height = 20;
                    config->total_mines = 50;
                }
            } else if (selection == 1) {
                config->variant = (config->variant + 1) % 3;
            }
        } else if (key == NCKEY_ENTER || key == NCKEY_SPACE) {
            if (selection == 2) {
                break;
            }
        } else if (config->size_mode == MODE_CUSTOM && selection == 0) {
            if (key == 'q' && config->width > 5) config->width--;
            if (key == 'w' && config->width < (int)dimx / 2 - 2) config->width++; // Approximate max width
            if (key == 'a' && config->height > 5) config->height--;
            if (key == 's' && config->height < (int)dimy - 4) config->height++; // Approximate max height
            if (key == 'z' && config->total_mines > 1) config->total_mines--;
            if (key == 'x' && config->total_mines < 1000 && config->total_mines < config->widght * config->height - 1) config->total_mines++
        }
    }
    apply_size_mode(config);
}

/**
 * @brief Initialize the board state
 *
 * @param board The board to initialize
 * @param config The game configuration
 */
 void init_board(Board *board, GameConfig *config) {
    board->width = config->width;
    board->height = config->height;
    board->total_mines = config->total_mines;
    board->flagged_count = 0;
    board->revealed_count = 0;
    board->game_over = false;
    board->victory = false;
    board->cursor_x = board->width / 2;
    board->cursor_y = board->height / 2;
    board->first_click = true;

    board->grid = malloc(board->height * sizeof(Cell*));
    for (int y = 0; y < board->height; y++) {
        board->grid[y] = malloc(board->width * sizeof(Cell));
        for (int x = 0; x < board->width; x++) {
            board->grid[y][x].is_mine = false;
            board->grid[y][x].is_revealed = false;
            board->grid[y][x].is_flagged = false;
            board->grid[y][x].adjacent_mines = 0;
            board->grid[y][x].actual_adjacent = 0;
        }
    }
 }

 /**
  * @brief Free the dynamically allocated board
  *
  * @param board Free the board
  */
void free_board(Board *board) {
    if (board->grid != NULL) {
        for (int y = 0; y < board->height; y++) {
            free (board->grid[y]);
        }
        free(board->grid);
        board->grid = NULL;
    }
}

/**
 * @brief Calculate adjacent mines
 *
 * @param board Game board
 * @param config Game Configuration
 */
void calculate_adjacent_mines(Board *board, GameConfig *config) {
    int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

    for (int y = 0; y < board->height; y++) {
        for (int x = 0; x < board->width; x++) {
            if(board->grid[y][x].is_mine) continue;

            int count = 0;
            for (int i = 0; i < 8; i++) {
                int nx = x + dx[i];
                int ny = y + dy[i];

                if (nx >= 0 && nx < board->width && ny >= 0 && ny < board->height) {
                    if (board->grid[ny][nx].is_mine) {
                        if (config->variant == VARIANT_CHECKERBOARD) {
                            // In checkerboard, bombs on some tiles count as 2
                            // Even tiles count as 2
                            if ((nx + ny) % 2 == 0) {
                                count += 2;
                            } else {
                                count += 1;
                            }
                        } else {
                            count++;
                        }
                    }
                }
            }
            board->grid[y][x].actual_adjacent = count;

            if (config->variant == VARIANT_LIAR) {
                // Liar: if adjacent to mines, count is off by +-1 (not below 0)
                if (count > 0) {
                    int offset = (rand() % 2 == 0) ? 1 : -1;
                    int displayed = count + offset;
                    if (displayed < 0) displayed = 0;
                    board->grid[y][x].adjacent_mines = displayed;
                } else{
                    board->grid[y][x].adjacent_mines = count;
                }
            } else {
                board->grid[y][x].adjacent_mines = count;
            }
        }
    }
}