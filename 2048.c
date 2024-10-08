#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// includes Windows system functions to refresh the game interface smoothly
#include <windows.h>

// includes the non-standard _getch() function to read keyboard inputs without echo
#include <conio.h>


#define GRID_SIZE 4


int score;
int grid[GRID_SIZE][GRID_SIZE];


bool enable_virtual_terminal_sequences(void);
void show_error_message(void);
void pause_game(void);
void start_game(void);
void initialize_game(void);
void generate_tile(bool is_tile_value_random);
int count_empty_spots(void);
int generate_tile_value(bool is_tile_value_random);
void refresh_game_interface(void);
bool check_game_status(void);
void read_keyboard_input(void);
void move_tiles_up(void);
void move_tiles_down(void);
void move_tiles_left(void);
void move_tiles_right(void);
void show_gameplay(void);
void show_ending_message(void);


int main(void)
{
    if (!enable_virtual_terminal_sequences())
    {
        show_error_message();
        return -1;
    }

    // hides the cursor with a specific virtual terminal sequence
    printf("\x1B[?25l");

    start_game();

    return 0;
}


/*
<Summary> :: enables virtual terminal sequences of console output to refresh the game interface smoothly
<Parameters> :: none
<Return> :: whether the activation of virtual terminal sequences succeeds
*/
bool enable_virtual_terminal_sequences(void)
{
    HANDLE output_handle;
    DWORD output_mode;

    // gets the handle of console output, and exits the current function if the handle is not obtained successfully
    output_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (output_handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    // gets the current output mode, and exits the current function if the mode is not obtained successfully
    if (!GetConsoleMode(output_handle, &output_mode))
    {
        return false;
    }

    // sets the output mode to process virtual terminal sequences, and exits the current function if the activation fails
    output_mode |= (DWORD) 4;

    if (!SetConsoleMode(output_handle, output_mode))
    {
        return false;
    }

    return true;
}


/*
<Summary> :: shows a message about the activation failure of virtual terminal sequences in the console,
             and pauses the game until Enter is pressed
<Parameters> :: none
<Return> :: none
*/
void show_error_message(void)
{
    printf("[Error] The activation of virtual terminal sequences failed!\n\n");
    printf("Press Enter to close the game.....");

    pause_game();

    return;
}


/*
<Summary> :: pauses the game until Enter is pressed
<Parameters> :: none
<Return> :: none
*/
void pause_game(void)
{
    bool is_enter_pressed = false;

    while (!is_enter_pressed)
    {
        // reads a keyboard input without echo, and jumps out of the loop if Enter is pressed
        if (_getch() == 13)
        {
            is_enter_pressed = true;
        }
    }

    return;
}


/*
<Summary> :: starts the main cycle of the game
<Parameters> :: none
<Return> :: none
*/
void start_game(void)
{
    while (true)
    {
        bool is_game_over = false;

        initialize_game();

        while (!is_game_over)
        {
            refresh_game_interface();
            is_game_over = check_game_status();

            if (!is_game_over)
            {
                read_keyboard_input();
            }
            else
            {
                show_ending_message();
            }
        }

        // clears the console with a Windows-specific command
        system("cls");
    }

    return;
}


/*
<Summary> :: initializes the score and grid for a new game
<Parameters> :: none
<Return> :: none
*/
void initialize_game(void)
{
    score = 0;

    for (int row = 0; row < GRID_SIZE; ++row)
    {
        for (int column = 0; column < GRID_SIZE; ++column)
        {
            grid[row][column] = 0;
        }
    }

    generate_tile(false);
    generate_tile(true);

    return;
}


/*
<Summary> :: generates a new tile with initial value at a random empty spot in the grid
<Parameter "is_tile_value_random"> :: the initial value would be 2 (90% probability) or 4 (10% probability)
                                      when this parameter is true; otherwise the initial value would be 2
<Return> :: none
*/
void generate_tile(bool is_tile_value_random)
{
    int index_of_new_tile;

    srand((unsigned int) time(NULL));
    index_of_new_tile = rand() % count_empty_spots();

    for (int row = 0; row < GRID_SIZE; ++row)
    {
        for (int column = 0; column < GRID_SIZE; ++column)
        {
            if (grid[row][column] == 0 && index_of_new_tile != 0)
            {
                --index_of_new_tile;
                continue;
            }

            if (grid[row][column] == 0 && index_of_new_tile == 0)
            {
                grid[row][column] = generate_tile_value(is_tile_value_random);
                return;
            }
        }
    }
}


/*
<Summary> :: counts the empty spots in the grid
<Parameters> :: none
<Return> :: the number of empty spots in the grid
*/
int count_empty_spots(void)
{
    int total_empty_spots = 0;

    for (int row = 0; row < GRID_SIZE; ++row)
    {
        for (int column = 0; column < GRID_SIZE; ++column)
        {
            if (grid[row][column] == 0)
            {
                ++total_empty_spots;
            }
        }
    }

    return total_empty_spots;
}


/*
<Summary> :: generates an initial value of a new tile
<Parameter "is_tile_value_random"> :: the initial value would be 2 (90% probability) or 4 (10% probability)
                                      when this parameter is true; otherwise the initial value would be 2
<Return> :: an integer 2 or 4
*/
int generate_tile_value(bool is_tile_value_random)
{
    int tile_value;

    if (is_tile_value_random)
    {
        tile_value = (rand() % 10) ? 2 : 4;
    }
    else
    {
        tile_value = 2;
    }

    return tile_value;
}


/*
<Summary> :: shows the current score, grid, and some instructions in the console
<Parameters> :: none
<Return> :: none
*/
void refresh_game_interface(void)
{
    // moves the cursor to the beginning of second line with a specific virtual terminal sequence
    printf("\x1B[2;1H");

    printf("  Score: %d\n\n", score);
    printf("  |===================================|\n");

    for (int row = 0; row < GRID_SIZE; ++row)
    {
        printf("  |        |        |        |        |\n  ");

        for (int column = 0; column < GRID_SIZE; ++column)
        {
            if (grid[row][column] == 0)
            {
                printf("|        ");
            }
            else
            {
                printf("| %6d ", grid[row][column]);
            }
        }

        printf("|\n  |        |        |        |        |\n");

        if (row < GRID_SIZE - 1)
        {
            printf("  |--------+--------+--------+--------|\n");
        }
    }

    printf("  |===================================|\n\n");
    printf("  [Arrow Keys] Move the tiles   [Esc] Show the gameplay");

    return;
}


/*
<Summary> :: checks whether there are still empty spots or adjacent tiles with the same value in the grid
<Parameters> :: none
<Return> :: whether the game is over
*/
bool check_game_status(void)
{
    if (count_empty_spots() != 0)
    {
        return false;
    }

    for (int row = 0; row < GRID_SIZE; ++row)
    {
        for (int column = 0; column < GRID_SIZE - 1; ++column)
        {
            if (grid[row][column] == grid[row][column + 1])
            {
                return false;
            }
        }
    }

    for (int column = 0; column < GRID_SIZE; ++column)
    {
        for (int row = 0; row < GRID_SIZE - 1; ++row)
        {
            if (grid[row][column] == grid[row + 1][column])
            {
                return false;
            }
        }
    }

    return true;
}


/*
<Summary> :: reads a keyboard input without echo to move tiles or show the gameplay page
<Parameters> :: none
<Return> :: none
*/
void read_keyboard_input(void)
{
    switch (_getch())
    {
        // the first byte of extended key codes for special keys
        case 0:
        case 224:
            switch (_getch())
            {
                case 72: // the second byte of the extended key code for Upward Arrow
                    move_tiles_up();
                    break;
                case 80: // the second byte of the extended key code for Downward Arrow
                    move_tiles_down();
                    break;
                case 75: // the second byte of the extended key code for Leftward Arrow
                    move_tiles_left();
                    break;
                case 77: // the second byte of the extended key code for Rightward Arrow
                    move_tiles_right();
                    break;
                default: // the second byte of extended key codes that do not correspond to any valid operations
                    break;
            }
            break;

        // the key code for Esc
        case 27:
            show_gameplay();
            break;

        // the key codes that do not correspond to any valid operations
        default:
            break;
    }

    return;
}


/*
<Summary> :: moves all the tiles in the grid upwards, and generates a new tile at a random empty spot if the grid changes
<Parameters> :: none
<Return> :: none
*/
void move_tiles_up(void)
{
    bool has_grid_changed = false;

    // moves the tiles in each column upwards except for those in the first row
    for (int column = 0; column < GRID_SIZE; ++column)
    {
        int row_of_collided_tile = 0;
        bool has_collided_tile_merged = false;

        for (int row = 1; row < GRID_SIZE; ++row)
        {
            if (grid[row][column] != 0)
            {
                // the moved tile collides with the edge of grid
                if (grid[row_of_collided_tile][column] == 0)
                {
                    grid[row_of_collided_tile][column] = grid[row][column];
                    grid[row][column] = 0;
                    has_grid_changed = true;
                }
                // the moved tile collides with another tile and they merge
                else if (grid[row_of_collided_tile][column] == grid[row][column] && !has_collided_tile_merged)
                {
                    grid[row_of_collided_tile][column] *= 2;
                    grid[row][column] = 0;
                    score += grid[row_of_collided_tile][column];
                    has_collided_tile_merged = true;
                    has_grid_changed = true;
                }
                // the moved tile collides with another tile and they do not merge
                else
                {
                    if (++row_of_collided_tile != row)
                    {
                        grid[row_of_collided_tile][column] = grid[row][column];
                        grid[row][column] = 0;
                        has_grid_changed = true;
                    }

                    has_collided_tile_merged = false;
                }
            }
        }
    }

    if (has_grid_changed)
    {
        generate_tile(true);
    }

    return;
}


/*
<Summary> :: moves all the tiles in the grid downwards, and generates a new tile at a random empty spot if the grid changes
<Parameters> :: none
<Return> :: none
*/
void move_tiles_down(void)
{
    bool has_grid_changed = false;

    // moves the tiles in each column downwards except for those in the last row
    for (int column = 0; column < GRID_SIZE; ++column)
    {
        int row_of_collided_tile = GRID_SIZE - 1;
        bool has_collided_tile_merged = false;

        for (int row = GRID_SIZE - 2; row >= 0; --row)
        {
            if (grid[row][column] != 0)
            {
                // the moved tile collides with the edge of grid
                if (grid[row_of_collided_tile][column] == 0)
                {
                    grid[row_of_collided_tile][column] = grid[row][column];
                    grid[row][column] = 0;
                    has_grid_changed = true;
                }
                // the moved tile collides with another tile and they merge
                else if (grid[row_of_collided_tile][column] == grid[row][column] && !has_collided_tile_merged)
                {
                    grid[row_of_collided_tile][column] *= 2;
                    grid[row][column] = 0;
                    score += grid[row_of_collided_tile][column];
                    has_collided_tile_merged = true;
                    has_grid_changed = true;
                }
                // the moved tile collides with another tile and they do not merge
                else
                {
                    if (--row_of_collided_tile != row)
                    {
                        grid[row_of_collided_tile][column] = grid[row][column];
                        grid[row][column] = 0;
                        has_grid_changed = true;
                    }

                    has_collided_tile_merged = false;
                }
            }
        }
    }

    if (has_grid_changed)
    {
        generate_tile(true);
    }

    return;
}


/*
<Summary> :: moves all the tiles in the grid leftwards, and generates a new tile at a random empty spot if the grid changes
<Parameters> :: none
<Return> :: none
*/
void move_tiles_left(void)
{
    bool has_grid_changed = false;

    // moves the tiles in each row leftwards except for those in the first column
    for (int row = 0; row < GRID_SIZE; ++row)
    {
        int column_of_collided_tile = 0;
        bool has_collided_tile_merged = false;

        for (int column = 1; column < GRID_SIZE; ++column)
        {
            if (grid[row][column] != 0)
            {
                // the moved tile collides with the edge of grid
                if (grid[row][column_of_collided_tile] == 0)
                {
                    grid[row][column_of_collided_tile] = grid[row][column];
                    grid[row][column] = 0;
                    has_grid_changed = true;
                }
                // the moved tile collides with another tile and they merge
                else if (grid[row][column_of_collided_tile] == grid[row][column] && !has_collided_tile_merged)
                {
                    grid[row][column_of_collided_tile] *= 2;
                    grid[row][column] = 0;
                    score += grid[row][column_of_collided_tile];
                    has_collided_tile_merged = true;
                    has_grid_changed = true;
                }
                // the moved tile collides with another tile and they do not merge
                else
                {
                    if (++column_of_collided_tile != column)
                    {
                        grid[row][column_of_collided_tile] = grid[row][column];
                        grid[row][column] = 0;
                        has_grid_changed = true;
                    }

                    has_collided_tile_merged = false;
                }
            }
        }
    }

    if (has_grid_changed)
    {
        generate_tile(true);
    }

    return;
}


/*
<Summary> :: moves all the tiles in the grid rightwards, and generates a new tile at a random empty spot if the grid changes
<Parameters> :: none
<Return> :: none
*/
void move_tiles_right(void)
{
    bool has_grid_changed = false;

    // moves the tiles in each row rightwards except for those in the last column
    for (int row = 0; row < GRID_SIZE; ++row)
    {
        int column_of_collided_tile = GRID_SIZE - 1;
        bool has_collided_tile_merged = false;

        for (int column = GRID_SIZE - 2; column >= 0; --column)
        {
            if (grid[row][column] != 0)
            {
                // the moved tile collides with the edge of grid
                if (grid[row][column_of_collided_tile] == 0)
                {
                    grid[row][column_of_collided_tile] = grid[row][column];
                    grid[row][column] = 0;
                    has_grid_changed = true;
                }
                // the moved tile collides with another tile and they merge
                else if (grid[row][column_of_collided_tile] == grid[row][column] && !has_collided_tile_merged)
                {
                    grid[row][column_of_collided_tile] *= 2;
                    grid[row][column] = 0;
                    score += grid[row][column_of_collided_tile];
                    has_collided_tile_merged = true;
                    has_grid_changed = true;
                }
                // the moved tile collides with another tile and they do not merge
                else
                {
                    if (--column_of_collided_tile != column)
                    {
                        grid[row][column_of_collided_tile] = grid[row][column];
                        grid[row][column] = 0;
                        has_grid_changed = true;
                    }

                    has_collided_tile_merged = false;
                }
            }
        }
    }

    if (has_grid_changed)
    {
        generate_tile(true);
    }

    return;
}


/*
<Summary> :: shows the gameplay in the console, and pauses the game until Enter is pressed
<Parameters> :: none
<Return> :: none
*/
void show_gameplay(void)
{
    system("cls");

    printf("\n");
    printf("  [HOW TO PLAY]\n");
    printf("\n");
    printf("  2048 is played on a 4x4 grid, with numbered tiles that slide when a player presses one of the arrow keys.\n");
    printf("  Tiles slide as far as possible along the chosen direction until they are stopped by another tile or the\n");
    printf("  edge of grid. If two tiles with the same number collide while moving, they will merge into a tile with\n");
    printf("  the sum of the two tiles that collided. The resulting tile cannot merge with another tile again in the\n");
    printf("  same move. Every time the grid changes, a new tile numbered 2 or 4 randomly appears at an empty spot in\n");
    printf("  the grid.\n");
    printf("\n");
    printf("  The player's score is recorded in the top left and increased whenever two tiles combine, by the value of\n");
    printf("  the resulting tile. When the player has no legal moves (there are no empty spots and no adjacent tiles\n");
    printf("  with the same value), the game ends.\n");
    printf("\n\n");
    printf("  Press Enter to continue playing.....");

    pause_game();

    system("cls");

    return;
}


/*
<Summary> :: shows the ending message in the console, and pauses the game until Enter is pressed
<Parameters> :: none
<Return> :: none
*/
void show_ending_message(void)
{
    printf("\n\n\n  There are no legal moves. Press Enter to start a new game.....");

    pause_game();

    return;
}