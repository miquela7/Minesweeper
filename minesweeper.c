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
    int actual_adjacent;     /**< Actual number of adjacent mines (for Liar) */
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
            config->total_mines = 10;
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
  * @brief Displays the introduction screen and gets user input
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

        ncplane_putstr_yx(stdn, 4, 4, selection == 0 ? "> Size Mode: " : " Size Mode: ");
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
        switch (config->variant) {
            case VARIANT_NORMAL: var_str = "Normal"; break;
            case VARIANT_CHECKERBOARD: var_str = "Checkerboard"; break;
            case VARIANT_LIAR: var_str = "The Liar"; break;
        }
        ncplane_putstr(stdn, var_str);

        ncplane_putstr_yx(stdn, y_offset + 2, 4, selection == 2 ? "> Start Game" : " Start Game");
        ncplane_putstr_yx(stdn, dimy - 2, 2, "Use UP/DOWN to navigate, LEFT/RIGHT to change options, Enter to start.");

        notcurses_render(nc);

        uint32_t key = notcurses_get_blocking(nc, &ni);
        if (key == (uint32_t)-1) continue;
        if (ni.evtype != NCTYPE_PRESS) continue;

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
        }else if (key == NCKEY_RIGHT || key == 'l') {
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
            if (key == 'x' && config->total_mines < 1000 && config->total_mines < config->width * config->height - 1) config->total_mines++;
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

/**
 * @brief Place mines on board randomly
 *
 * @param board Game board
 * @param config Game Configuration
 * @param first_click_x X coordinate of the first click
 * @param first_click_y Y coordinate of the first click
 */
void place_mines(Board *board, GameConfig *config, int first_click_x, int first_click_y) {
    int mines_to_place = board->total_mines;

    //Ensure the placement of mines on valid cells
    int safe_cells = 9; //Max 9 cells around first click are safe
    int max_mines = (board->width * board->height) - safe_cells;
    if (mines_to_place > max_mines) {
        mines_to_place = max_mines;
        board->total_mines = max_mines;
    }

    while (mines_to_place > 0) {
        int x = rand() % board->width;
        int y = rand() % board->height;

        //Ensure first click and surroundings are safe
        if (abs(x - first_click_x) <= 1 && abs(y - first_click_y) <= 1) {
            continue;
        }

        if (!board->grid[y][x].is_mine) {
            board->grid[y][x].is_mine = true;
            mines_to_place--;
        }
    }

    calculate_adjacent_mines(board, config);
}

/**
 * @brief Renders the game board to terminal
 * 
 * @param nc notcurses text
 * @param board game board state
 * @param config game configuration
 */
void render_board(struct notcurses *nc, Board *board, GameConfig *config) {
    struct ncplane* stdn = notcurses_stdplane(nc);
    ncplane_erase(stdn);

    unsigned int dimy, dimx;
    ncplane_dim_yx(stdn, &dimy, &dimx);

    //Calculate centering offsets
    int start_y = ((int)dimy - board->height) / 2;
    int start_x = ((int)dimx - (board->width * 2)) / 2; // * 2, each cell is 2 chars wide

    if (start_y < 2) start_y = 2;
    if (start_x < 0) start_x = 0;

    // Header
    ncplane_printf_yx(stdn, start_y - 2, start_x, "Total Mines: %d | Remaining: %d",
                      board->total_mines, board->total_mines - board->flagged_count);
    
    if (board->game_over) {
        if (board->victory) {
            ncplane_putstr_yx(stdn, start_y - 2, start_x + 40, "You Win!");
        } else {
            ncplane_putstr_yx(stdn, start_y - 2, start_x + 40, "Game Over!");
        }
    }

    //Grid
    for (int y = 0; y < board->height; y++) {
        for (int x = 0; x < board->width; x++) {
            Cell *cell = &board->grid[y][x];
            int screen_y = start_y +y;
            int screen_x = start_x + (x * 2);

            uint32_t bg_color = 0x222222; // Dark grey default
            if (config->variant == VARIANT_CHECKERBOARD) {
                if ((x + y) % 2 == 0) {
                    bg_color = 0x333333; // lighter grey
                }
            }
            if (board->cursor_x == x && board->cursor_y == y) {
                bg_color = 0x666666; // Highlight cursor
            }

            ncplane_set_bg_rgb(stdn, bg_color);

            if (cell->is_revealed || (board->game_over && cell->is_mine)) {
                if (cell->is_mine) {
                    ncplane_set_fg_rgb(stdn, 0xFF0000); //Red bomb
                    ncplane_printf_yx(stdn, screen_y, screen_x, " *");
                } else if (cell->adjacent_mines > 0) {
                    // Distinct color for numbers
                    uint32_t fg_color;
                    switch (cell->adjacent_mines) {
                        case 1: fg_color = 0x0000FF; break; // Blue
                        case 2: fg_color = 0x008000; break; // Green
                        case 3: fg_color = 0xFF0000; break; // Red
                        case 4: fg_color = 0x000080; break; // Navy
                        case 5: fg_color = 0x800000; break; // Maroon
                        case 6: fg_color = 0x00FFFF; break; // Cyan 
                        case 7: fg_color = 0x800080; break; // Purple
                        case 8: fg_color = 0x808080; break; // Grey
                        default: fg_color = 0xFFFFFF; break; // White
                    }
                    ncplane_set_fg_rgb(stdn, fg_color);
                    ncplane_printf_yx(stdn, screen_y, screen_x, " %d", cell->adjacent_mines);
                } else {
                    ncplane_printf_yx(stdn, screen_y, screen_x, " ");
                }
            } else {
                if (cell->is_flagged) {
                    ncplane_set_fg_rgb(stdn, 0xFFA500); //Orange Flag
                    ncplane_printf_yx(stdn, screen_y, screen_x, " F");
                } else {
                    ncplane_set_fg_rgb(stdn, 0xFFFFFF);
                    ncplane_printf_yx(stdn, screen_y, screen_x, " .");
                }
            }
            ncplane_set_bg_default(stdn);
            ncplane_set_fg_default(stdn);
        }
    }

    ncplane_putstr_yx(stdn, (int)dimy - 1, 0, "WASD/Arrows to move, F to flag, C to clear, or Q to quit.");

    notcurses_render(nc);
}

/**
 * @brief checks if the player has won
 * 
 *@param board game board
 */
void check_win_condition(Board *board) {
    int total_cells = board->width * board->height;
    if (board->revealed_count == total_cells - board->total_mines) {
        board->game_over = true;
        board->victory = true;
    }
}

/**
 * @brief reveal empty cells
 *
 * @param board game board
 * @param x X coordinate
 * @param y Y coordinate
 */
void reveal_empty_cells(Board *board, int x, int y) {
    if (x < 0 || x>= board->width || y < 0 || y >= board->height) return;

    Cell *cell = &board->grid[y][x];
    if (cell->is_revealed || cell->is_flagged || cell->is_mine) return;

    cell->is_revealed = true;
    board->revealed_count++;

    if (cell->actual_adjacent == 0) {
        int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};
        for (int i = 0; i < 8; i++) {
            reveal_empty_cells(board, x + dx[i], y +dy[i]);
        }
    }
}

/**
 * @brief Clear action on a cell
 *
 * @param board game board
 * @param config game configuration
 * @param x X coordinate
 * @param y Y coordinate
 */
void handle_clear(Board *board, GameConfig *config, int x, int y) {
    if (x < 0 || x >= board->width || y < 0 || y >= board->height) return;
    if (board->game_over) return;

    Cell *cell = &board->grid[y][x];
    if (cell->is_flagged) return;

    if (board->first_click) {
        board->first_click = false;
        place_mines(board, config, x, y);
    }
    
    if (cell->is_mine) {
        board->game_over = true;
        board->victory = false;
        cell->is_revealed = true; // Show the bomb that ended the game
    } else if (!cell->is_revealed) {
        reveal_empty_cells(board, x, y);
        check_win_condition(board); 
    }
}

/** 
 * @brief Handle the flag action on a cell
 * 
 * @param board game board
 * @param x X coordinate
 * @param y Y coordinate
 */
void handle_flag(Board *board, int x, int y) {
    if (x < 0 || x >= board->width || y < 0 || y >= board->height) return;
    if (board->game_over) return; 

    Cell *cell= &board-> grid[y][x];
    if (cell->is_revealed) return;

    cell->is_flagged = !cell->is_flagged;
    if (cell->is_flagged) {
        board->flagged_count++;
    } else {
        board->flagged_count--;
    }
}

int main(void) {
    srand(time(NULL));
    struct notcurses_options opts = {0};
    opts.flags |= NCOPTION_SUPPRESS_BANNERS | NCOPTION_NO_QUIT_SIGHANDLERS;
    struct notcurses* nc = notcurses_core_init(&opts, NULL);
    if (nc == NULL) {
        return EXIT_FAILURE;
    }

    // Enable Mouse 
    notcurses_mice_enable(nc, NCMICE_ALL_EVENTS);

    GameConfig config;
    show_intro_screen(nc, &config);

    Board board;
    init_board(&board, &config);

    struct ncinput ni;
    bool quit = false;

    while (!quit) {
        render_board(nc, &board, &config);

        uint32_t key = notcurses_get_blocking(nc, &ni);
        if (key == (uint32_t)-1) continue;
        if (ni.evtype != NCTYPE_PRESS) continue;

        if (key == 'q' || key == 'Q') {
            quit = true;
        } else if (!board.game_over) {
            if (key == NCKEY_UP || key == 'w' || key == 'W') {
                if (board.cursor_y > 0) board.cursor_y--;
            } else if (key == NCKEY_DOWN || key == 's' || key == 'S') {
                if (board.cursor_y < board.height - 1) board.cursor_y++;
            } else if (key == NCKEY_LEFT || key == 'a' || key == 'A') {
                if (board.cursor_x > 0) board.cursor_x--;
            } else if (key == NCKEY_RIGHT || key == 'd' || key == 'D') {
                if (board.cursor_x < board.width -1) board.cursor_x++;
            } else if (key == 'c' || key == 'C' || key == NCKEY_ENTER) {
                handle_clear(&board, &config, board.cursor_x, board.cursor_y);
            } else if (key == 'f' || key == 'F') {
                handle_flag(&board, board.cursor_x, board.cursor_y);
            } else if (key == NCKEY_BUTTON1) {
                // Mouse left click : clear
                int start_y = ((int)ncplane_dim_y(notcurses_stdplane(nc)) - board.height) / 2;
                int start_x = ((int)ncplane_dim_x(notcurses_stdplane(nc)) - (board.width * 2)) / 2;
                if (start_y < 2) start_y = 2;
                if (start_x < 0) start_x = 0;

                int cy = ni.y - start_y;
                int cx = (ni.x - start_x - 3) / 2;
                if (cx >= 0 && cx < board.width && cy >= 0 && cy < board.height) {
                    board.cursor_x = cx;
                    board.cursor_y = cy;
                    handle_clear(&board, &config, cx, cy);
                }
            } else if (key == NCKEY_BUTTON3) {
                //Mouse click right: flag
                int start_y = ((int)ncplane_dim_y(notcurses_stdplane(nc)) - board.height) / 2;
                int start_x = ((int)ncplane_dim_x(notcurses_stdplane(nc)) - (board.width * 2)) / 2;
                if (start_y < 2) start_y = 2;
                if (start_x < 0) start_x = 0;

                int cy = ni.y - start_y;
                int cx = (ni.x - start_x - 3) / 2;
                if (cx >= 0 && cx < board.width && cy < board.height) {
                    board.cursor_x = cx;
                    board.cursor_y = cy;
                    handle_flag(&board, cx, cy);
                }
            }
        } else if (board.game_over) {
            if (key == 'c' || key == 'C') {
                free_board(&board);
                init_board(&board, &config);
            }
        }
    }

    free_board(&board);
    notcurses_stop(nc);
    return EXIT_SUCCESS;
}