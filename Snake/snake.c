#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// includes Windows API to perform OS-related tasks
#include <windows.h>

// includes the non-standard functions _getch() and _kbhit() to allow more flexible keyboard input
#include <conio.h>


enum keycode
{
    ENTER = 13,
    UP_ARROW = 72,
    DOWN_ARROW = 80,
    LEFT_ARROW = 75,
    RIGHT_ARROW = 77,
    Y_UPPER = 89,
    Y_LOWER = 121,
    N_UPPER = 78,
    N_LOWER = 110
};

enum snake_direction
{
    STILL,
    UP,
    DOWN,
    LEFT,
    RIGHT
};


struct snake_node
{
    int position[2];
    struct snake_node *ptr_next_node;
};


int current_score;
int best_score;
struct snake_node *ptr_snake_head;
enum snake_direction snake_movement;
bool is_snake_teleporting;
int teleport_destination[2];
int food_positions[2][2];


int set_up_console(void);
int adjust_console_size(void);
int adjust_font_size(void);
int enable_virtual_terminal_sequences(void);
void show_error_message(int error_code);
int start_game(void);
int initialize_game(void);
int initialize_scores(void);
int initialize_snake(void);
void initialize_food(void);
void initialize_game_interface(void);
bool check_snake_state(void);
void locate_new_snake_head(int new_snake_head_position[2]);
int refresh_game_interface(void);
int insert_new_snake_head(int new_snake_head_line, int new_snake_head_column);
void move_cursor(int new_line, int new_column);
void delete_snake_tail(void);
void generate_food(void);
bool check_food_collision(void);
void update_current_score(void);
void update_snake_direction(void);
int end_game(bool *ptr_game_state);
void display_dead_snake(void);
int update_record(void);
bool check_next_game(void);
void free_snake(void);


int main(void)
{
    int error_code;

    if ((error_code = set_up_console()))
    {
        show_error_message(error_code);
        return -1;
    }

    if ((error_code = start_game()))
    {
        show_error_message(error_code);
        free_snake();
        return -1;
    }

    return 0;
}


/*
<Summary> :: sets console properties before the main cycle of the game
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be
            the line number where the error occurs
*/
int set_up_console(void)
{
    int error_code;

    if ((error_code = adjust_console_size()))
    {
        return error_code;
    }

    if ((error_code = adjust_font_size()))
    {
        return error_code;
    }

    if ((error_code = enable_virtual_terminal_sequences()))
    {
        return error_code;
    }

    // changes the console title with a virtual terminal sequence
    printf("\x1B]0;Snake\x07");

    // hides the console cursor with a virtual terminal sequence
    printf("\x1B[?25l");

    return 0;
}


/*
<Summary> :: adjusts the screen buffer and window of the console to a proper size
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be
            the line number where the error occurs
*/
int adjust_console_size(void)
{
    HANDLE console_handle;
    COORD screen_buffer_size;
    SMALL_RECT window_size;

    // gets the console handle, and exits the current function if the handle is not obtained successfully
    console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (console_handle == INVALID_HANDLE_VALUE)
    {
        return __LINE__;
    }

    // assigns values to the structures that represent the new screen buffer size and window size
    screen_buffer_size.X = 70;
    screen_buffer_size.Y = 30;

    window_size.Top = 0;
    window_size.Bottom = screen_buffer_size.Y - 1;
    window_size.Left = 0;
    window_size.Right = screen_buffer_size.X - 1;

    // sets the window and screen buffer to the new size, and exits the current function if any adjustment fails
    if (!SetConsoleWindowInfo(console_handle, TRUE, &window_size))
    {
        return __LINE__;
    }

    if (!SetConsoleScreenBufferSize(console_handle, screen_buffer_size))
    {
        return __LINE__;
    }

    return 0;
}


/*
<Summary> :: adjusts the console font to a proper size
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be
            the line number where the error occurs
*/
int adjust_font_size(void)
{
    HANDLE console_handle;
    CONSOLE_FONT_INFOEX font_information;

    // gets the console handle, and exits the current function if the handle is not obtained successfully
    console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (console_handle == INVALID_HANDLE_VALUE)
    {
        return __LINE__;
    }

    // gets the information about the current font, and exits the current function if the information is not obtained successfully
    font_information.cbSize = sizeof(CONSOLE_FONT_INFOEX);

    if (!GetCurrentConsoleFontEx(console_handle, FALSE, &font_information))
    {
        return __LINE__;
    }

    // sets the console font to the new size, and exits the current function if the adjustment fails
    font_information.dwFontSize.Y = 18;

    if (!SetCurrentConsoleFontEx(console_handle, FALSE, &font_information))
    {
        return __LINE__;
    }

    return 0;
}


/*
<Summary> :: enables virtual terminal sequences of console output
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be
            the line number where the error occurs
*/
int enable_virtual_terminal_sequences(void)
{
    HANDLE console_handle;
    DWORD output_mode;

    // gets the console handle, and exits the current function if the handle is not obtained successfully
    console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (console_handle == INVALID_HANDLE_VALUE)
    {
        return __LINE__;
    }

    // gets the current output mode, and exits the current function if the mode is not obtained successfully
    if (!GetConsoleMode(console_handle, &output_mode))
    {
        return __LINE__;
    }

    // sets the output mode to process virtual terminal sequences, and exits the current function if the activation fails
    output_mode |= 0x0004;

    if (!SetConsoleMode(console_handle, output_mode))
    {
        return __LINE__;
    }

    return 0;
}


/*
<Summary> :: replaces the contents in the console with an error message, and pauses the game until Enter is pressed
<Parameter "error_code"> :: the line number where the last error occurs
<Return> :: none
*/
void show_error_message(int error_code)
{
    // clears the console and moves the cursor to the initial position with a Windows-specific command
    system("cls");

    printf("[Error] The program terminates at the line %d!\n\n", error_code);
    printf("Press Enter to close the game.....");

    // waits for unbuffered keyboard input without echo, and jumps out of the loop if Enter is pressed
    while (_getch() != ENTER);

    return;
}


/*
<Summary> :: starts the main cycle of the game
<Parameters> :: none
<Return> :: the return value would be 0 if the game ends normally; otherwise the return value would be
            the line number where the error occurs
*/
int start_game(void)
{
    bool is_game_running = true;

    while (is_game_running)
    {
        int error_code;

        if ((error_code = initialize_game()))
        {
            return error_code;
        }

        while (check_snake_state())
        {
            if ((error_code = refresh_game_interface()))
            {
                return error_code;
            }

            // suspends the execution of the current thread for 50 milliseconds
            Sleep(50);

            update_snake_direction();
        }

        if ((error_code = end_game(&is_game_running)))
        {
            return error_code;
        }
    }

    return 0;
}


/*
<Summary> :: initializes the scores, snake, food for a new game and displays the initial game interface in the console
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be
            the line number where the error occurs
*/
int initialize_game(void)
{
    int error_code;

    if ((error_code = initialize_scores()))
    {
        return error_code;
    }

    if ((error_code = initialize_snake()))
    {
        return error_code;
    }

    initialize_food();

    initialize_game_interface();

    return 0;
}


/*
<Summary> :: resets the score for a new game, and reads the best score from a record file
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be
            the line number where the error occurs
*/
int initialize_scores(void)
{
    FILE *ptr_record_file;

    current_score = 0;

    ptr_record_file = fopen("best_record.txt", "r");

    if (ptr_record_file == NULL)
    {
        return __LINE__;
    }

    fscanf(ptr_record_file, "%d", &best_score);

    if (fclose(ptr_record_file) == EOF)
    {
        return __LINE__;
    }

    return 0;
}


/*
<Summary> :: creates a new snake that stays still at the initial position
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be
            the line number where the error occurs
*/
int initialize_snake(void)
{
    for (int column = 24; column < 29; ++column)
    {
        struct snake_node *ptr_snake_node = malloc(sizeof(struct snake_node));

        if (ptr_snake_node == NULL)
        {
            return __LINE__;
        }

        ptr_snake_node->position[0] = 12;
        ptr_snake_node->position[1] = column;

        if (column == 24)
        {
            ptr_snake_node->ptr_next_node = NULL;
            ptr_snake_head = ptr_snake_node;
        }
        else
        {
            ptr_snake_node->ptr_next_node = ptr_snake_head;
            ptr_snake_head = ptr_snake_node;
        }
    }

    snake_movement = STILL;
    is_snake_teleporting = false;

    return 0;
}


/*
<Summary> :: resets the food's position for a new game
<Parameters> :: none
<Return> :: none
*/
void initialize_food(void)
{
    food_positions[0][0] = 9;
    food_positions[0][1] = 41;
    food_positions[1][0] = 15;
    food_positions[1][1] = 41;

    return;
}


/*
<Summary> :: displays the initial interface for a new game in the console
<Parameters> :: none
<Return> :: none
*/
void initialize_game_interface(void)
{
    printf("\n");
    printf("                   Score: 0     Best: %d\n", best_score);
    printf("\n");

    // displays white walls, a colorful snake, and cyan food with virtual terminal sequences
    printf("                   \x1B[47m                                \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m                              \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m                              \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m                              \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m                              \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m                    \x1B[36mO\x1B[0m         \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m                              \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m                              \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m   \x1B[32m@@@@\x1B[93m@\x1B[0m                      \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m                              \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m                              \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m                    \x1B[36mO\x1B[0m         \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m                              \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m                              \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m                              \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m \x1B[0m                              \x1B[47m \x1B[0m\n");
    printf("                   \x1B[47m                                \x1B[0m\n");

    printf("\n\n");
    printf("     [HOW TO PLAY]\n");
    printf("     Control the movement of snake by Arrow Keys.\n");
    printf("     Try to eat more food and avoid hitting your tail or the wall.\n");

    return;
}


/*
<Summary> :: checks whether the snake will hit the wall or its tail in the next frame
<Parameters> :: none
<Return> :: whether the snake will be alive in the next frame
*/
bool check_snake_state(void)
{
    if (snake_movement != STILL && !is_snake_teleporting)
    {
        int new_snake_head_position[2];
        struct snake_node *ptr_snake_node;

        locate_new_snake_head(new_snake_head_position);

        if (new_snake_head_position[0] == 4 || new_snake_head_position[0] == 20)
        {
            return false;
        }
        if (new_snake_head_position[1] == 20 || new_snake_head_position[1] == 51)
        {
            return false;
        }

        ptr_snake_node = ptr_snake_head;

        while (ptr_snake_node->ptr_next_node != NULL)
        {
            if (ptr_snake_node->position[0] == new_snake_head_position[0])
            {
                if (ptr_snake_node->position[1] == new_snake_head_position[1])
                {
                    return false;
                }
            }

            ptr_snake_node = ptr_snake_node->ptr_next_node;
        }
    }

    return true;
}


/*
<Summary> :: stores the snake head position in the next frame to a specified array when the snake is moving normally
<Parameter "new_snake_head_position"> :: an array for storing the snake head position in the next frame
<Return> :: none
*/
void locate_new_snake_head(int new_snake_head_position[2])
{
    switch (snake_movement)
    {
        case UP:
            new_snake_head_position[0] = ptr_snake_head->position[0] - 1;
            new_snake_head_position[1] = ptr_snake_head->position[1];
            break;
        case DOWN:
            new_snake_head_position[0] = ptr_snake_head->position[0] + 1;
            new_snake_head_position[1] = ptr_snake_head->position[1];
            break;
        case LEFT:
            new_snake_head_position[0] = ptr_snake_head->position[0];
            new_snake_head_position[1] = ptr_snake_head->position[1] - 1;
            break;
        case RIGHT:
            new_snake_head_position[0] = ptr_snake_head->position[0];
            new_snake_head_position[1] = ptr_snake_head->position[1] + 1;
            break;
        default:
            break;
    }

    return;
}


/*
<Summary> :: updates the snake position based on the state of movement, and checks food collision or generates new food
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be
            the line number where the error occurs
*/
int refresh_game_interface(void)
{
    int error_code;

    if (is_snake_teleporting)
    {
        if ((error_code = insert_new_snake_head(teleport_destination[0], teleport_destination[1])))
        {
            return error_code;
        }

        delete_snake_tail();
        is_snake_teleporting = false;
        generate_food();
    }
    else if (snake_movement != STILL)
    {
        int new_snake_head_position[2];

        locate_new_snake_head(new_snake_head_position);

        if ((error_code = insert_new_snake_head(new_snake_head_position[0], new_snake_head_position[1])))
        {
            return error_code;
        }

        if (check_food_collision())
        {
            update_current_score();

            // plays asynchronous sound effects for eating food
            PlaySound(TEXT("eating_food.wav"), NULL, SND_FILENAME | SND_ASYNC);
        }
        else
        {
            delete_snake_tail();
        }
    }

    return 0;
}


/*
<Summary> :: inserts a new snake head for the next frame based on the specified position
<Parameter "new_snake_head_line"> :: the line where the snake head is located in the next frame
<Parameter "new_snake_head_column"> :: the column where the snake head is located in the next frame
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be
            the line number where the error occurs
*/
int insert_new_snake_head(int new_snake_head_line, int new_snake_head_column)
{
    struct snake_node *ptr_new_snake_head;

    // displays a yellow snake head at the new position and sets the old one to be green, with virtual terminal sequences
    move_cursor(new_snake_head_line, new_snake_head_column);
    printf("\x1B[93m@\x1B[0m");
    move_cursor(ptr_snake_head->position[0], ptr_snake_head->position[1]);
    printf("\x1B[32m@\x1B[0m");

    ptr_new_snake_head = malloc(sizeof(struct snake_node));

    if (ptr_new_snake_head == NULL)
    {
        return __LINE__;
    }

    ptr_new_snake_head->position[0] = new_snake_head_line;
    ptr_new_snake_head->position[1] = new_snake_head_column;
    ptr_new_snake_head->ptr_next_node = ptr_snake_head;
    ptr_snake_head = ptr_new_snake_head;

    return 0;
}


/*
<Summary> :: moves the cursor to the specified position
<Parameter "new_line"> :: the new line where the cursor is located
<Parameter "new_column"> :: the new column where the cursor is located
<Return> :: none
*/
void move_cursor(int new_line, int new_column)
{
    char str_cursor_positioning[10];

    // moves the cursor to the specified line and column with a virtual terminal sequence
    snprintf(str_cursor_positioning, sizeof(str_cursor_positioning), "\x1B[%d;%dH", new_line, new_column);
    printf(str_cursor_positioning);

    return;
}


/*
<Summary> :: removes the symbol and node of the end of snake
<Parameters> :: none
<Return> :: none
*/
void delete_snake_tail(void)
{
    struct snake_node *ptr_snake_node = ptr_snake_head;
    struct snake_node *ptr_previous_node = NULL;

    while (ptr_snake_node->ptr_next_node != NULL)
    {
        ptr_previous_node = ptr_snake_node;
        ptr_snake_node = ptr_snake_node->ptr_next_node;
    }

    if (ptr_snake_node->position[0] != ptr_snake_head->position[0] || ptr_snake_node->position[1] != ptr_snake_head->position[1])
    {
        move_cursor(ptr_snake_node->position[0], ptr_snake_node->position[1]);
        printf(" ");
    }

    free(ptr_snake_node);
    ptr_previous_node->ptr_next_node = NULL;

    return;
}


/*
<Summary> :: generates new food at two random empty positions
<Parameters> :: none
<Return> :: none
*/
void generate_food(void)
{
    int total_food_generated = 0;

    srand((unsigned int) time(NULL));

    while (total_food_generated < 2)
    {
        int random_line = rand() % 15 + 5;
        int random_column = rand() % 30 + 21;
        bool is_food_overlapped = false;
        struct snake_node *ptr_snake_node = ptr_snake_head;

        while (ptr_snake_node != NULL && !is_food_overlapped)
        {
            if (ptr_snake_node->position[0] == random_line && ptr_snake_node->position[1] == random_column)
            {
                is_food_overlapped = true;
            }

            ptr_snake_node = ptr_snake_node->ptr_next_node;
        }

        if (!is_food_overlapped)
        {
            if (food_positions[0][0] == random_line && food_positions[0][1] == random_column)
            {
                continue;
            }
            if (food_positions[1][0] == random_line && food_positions[1][1] == random_column)
            {
                continue;
            }

            move_cursor(random_line, random_column);

            // displays cyan food with a virtual terminal sequence
            printf("\x1B[36mO\x1B[0m");

            food_positions[total_food_generated][0] = random_line;
            food_positions[total_food_generated][1] = random_column;
            ++total_food_generated;
        }
    }

    return;
}


/*
<Summary> :: checks whether the snake eats any food, and sets up the teleport destination accordingly
<Parameters> :: none
<Return> :: whether the snake eats any food
*/
bool check_food_collision(void)
{
    bool is_food_eaten = false;

    if (ptr_snake_head->position[0] == food_positions[0][0] && ptr_snake_head->position[1] == food_positions[0][1])
    {
        is_food_eaten = true;
        is_snake_teleporting = true;
        teleport_destination[0] = food_positions[1][0];
        teleport_destination[1] = food_positions[1][1];
    }
    else if (ptr_snake_head->position[0] == food_positions[1][0] && ptr_snake_head->position[1] == food_positions[1][1])
    {
        is_food_eaten = true;
        is_snake_teleporting = true;
        teleport_destination[0] = food_positions[0][0];
        teleport_destination[1] = food_positions[0][1];
    }

    return is_food_eaten;
}


/*
<Summary> :: increases the current score by one and displays the updated score
<Parameters> :: none
<Return> :: none
*/
void update_current_score(void)
{
    ++current_score;

    // moves the cursor to the 27th column of the second line with a virtual terminal sequence
    printf("\x1B[2;27H");

    printf("%d", current_score);

    return;
}


/*
<Summary> :: updates the movement direction of snake if a valid keystroke is read from the console
<Parameters> :: none
<Return> :: none
*/
void update_snake_direction(void)
{
    bool is_snake_redirected = false;

    // keeps reading keystrokes from the console until the buffer is empty or the snake direction has been updated
    while (_kbhit() && !is_snake_redirected)
    {
        switch (_getch())
        {
            // the first byte of extended key codes for special keys
            case 0:
            case 224:
                switch (_getch())
                {
                    case UP_ARROW:
                        if (snake_movement == STILL || snake_movement == LEFT || snake_movement == RIGHT)
                        {
                            snake_movement = UP;
                            is_snake_redirected = true;
                        }
                        break;
                    case DOWN_ARROW:
                        if (snake_movement == STILL || snake_movement == LEFT || snake_movement == RIGHT)
                        {
                            snake_movement = DOWN;
                            is_snake_redirected = true;
                        }
                        break;
                    case LEFT_ARROW:
                        if (snake_movement == UP || snake_movement == DOWN)
                        {
                            snake_movement = LEFT;
                            is_snake_redirected = true;
                        }
                        break;
                    case RIGHT_ARROW:
                        if (snake_movement == STILL || snake_movement == UP || snake_movement == DOWN)
                        {
                            snake_movement = RIGHT;
                            is_snake_redirected = true;
                        }
                        break;
                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }

    return;
}


/*
<Summary> :: gives death feedback and asks whether player wants to play again, and updates the record file if necessary
<Parameter "ptr_game_state"> :: a pointer to the boolean indicating whether the game is still running
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be
            the line number where the error occurs
*/
int end_game(bool *ptr_game_state)
{
    int error_code;

    display_dead_snake();

    // plays asynchronous sound effects when the snake is dead
    PlaySound(TEXT("game_over.wav"), NULL, SND_FILENAME | SND_ASYNC);

    move_cursor(27, 1);

    if ((error_code = update_record()))
    {
        return error_code;
    }

    *ptr_game_state = check_next_game();

    free_snake();

    // clears the console and moves the cursor to the initial position with a Windows-specific command
    system("cls");

    return 0;
}


/*
<Summary> :: replaces the symbols of snake with red "X"s
<Parameters> :: none
<Return> :: none
*/
void display_dead_snake(void)
{
    struct snake_node *ptr_snake_node = ptr_snake_head;

    while (ptr_snake_node != NULL)
    {
        move_cursor(ptr_snake_node->position[0], ptr_snake_node->position[1]);

        // displays a red "X" with a virtual terminal sequence
        printf("\x1B[31mX\x1B[0m");

        ptr_snake_node = ptr_snake_node->ptr_next_node;
    }

    return;
}


/*
<Summary> :: writes the new best score to a record file if player breaks the record, and displays a congrats message
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be
            the line number where the error occurs
*/
int update_record(void)
{
    if (current_score > best_score)
    {
        FILE *ptr_record_file = fopen("best_record.txt", "w");

        if (ptr_record_file == NULL)
        {
            return __LINE__;
        }

        fprintf(ptr_record_file, "%d", current_score);

        if (fclose(ptr_record_file) == EOF)
        {
            return __LINE__;
        }

        // displays a bright yellow congrats message with a virtual terminal sequence
        printf("     \x1B[93mGreat! It's a new record.\x1B[0m\n");
    }

    return 0;
}


/*
<Summary> :: asks whether player wants to play again
<Parameters> :: none
<Return> :: whether the game will keep running
*/
bool check_next_game(void)
{
    bool is_game_running = true;
    bool is_choice_made = false;

    // displays a bright yellow message with a virtual terminal sequence to ask whether player wants to play again
    printf("     \x1B[93mDo yo want to play again.....(Y/N)\x1B[0m");

    while (!is_choice_made)
    {
        // waits for unbuffered keyboard input without echo to determine whether a new game will start
        switch (_getch())
        {
            case Y_UPPER:
            case Y_LOWER:
                is_choice_made = true;
                break;
            case N_UPPER:
            case N_LOWER:
                is_game_running = false;
                is_choice_made = true;
                break;
            // discards the next byte if the first byte of extended key codes for special keys is read
            case 0:
            case 224:
                _getch();
                break;
            default:
                break;
        }
    }

    return is_game_running;
}


/*
<Summary> :: releases the memory space allocated for the snake
<Parameters> :: none
<Return> :: none
*/
void free_snake(void)
{
    if (ptr_snake_head != NULL)
    {
        struct snake_node *ptr_snake_node = ptr_snake_head;

        while (ptr_snake_node != NULL)
        {
            struct snake_node *ptr_temp_node = ptr_snake_node->ptr_next_node;

            free(ptr_snake_node);

            ptr_snake_node = ptr_temp_node;
        }

        ptr_snake_head = NULL;
    }

    return;
}