#include <iostream>
#include <chrono>
#include <thread>

// includes Windows API to perform OS-related tasks
#include <windows.h>

// includes the non-standard _getch() function to read keyboard inputs without echo
#include <conio.h>


bool is_player_turn;
int gomoku_board[15][15];
int last_placed_row;
int last_placed_column;


int set_up_console();
int enable_mouse_input();
int adjust_console_size();
int adjust_font_size();
int enable_virtual_terminal_sequences();
void show_error_message(int error_code);
int start_game();
int show_title_screen();
void display_game_title();
void display_animated_hint(std::chrono::steady_clock::time_point &ref_animation_start_time);
int check_mouse_press(bool &ref_is_mouse_pressed);
void clear_console();
int determine_first_mover();
void initialize_selection_interface();
void move_cursor(int new_line, int new_column);
int get_mouse_position(POINT &ref_pixel_position_of_mouse);
void refresh_selection_interface(POINT pixel_position_of_mouse);
void highlight_left_card();
void highlight_right_card();
int check_card_selection(POINT pixel_position_of_mouse, bool &ref_is_card_selected);
void turn_over_left_card();
void turn_over_right_card();
int start_battle(bool &ref_is_game_running);
void initialize_battle();
int read_player_move();
int read_stone_placement(COORD &ref_character_position_of_click, int &ref_placed_row, int &ref_placed_column);
bool check_valid_placement(INPUT_RECORD input_record, COORD &ref_character_position_of_click, int &ref_placed_row, int &ref_placed_column);
void refresh_gomoku_board(COORD character_position_of_click);
int perform_ai_move();
void calculate_ai_move(COORD &ref_character_position_of_click, int &ref_placed_row, int &ref_placed_column);
bool check_neighbors(int row, int column);
double predict_board_value(bool is_player_next, int search_depth, double max_board_value, double min_board_value);
bool check_battle_state();
double assess_board_value();
double assess_stone_top(int row, int column);
double assess_stone_bottom(int row, int column);
double assess_stone_left(int row, int column);
double assess_stone_right(int row, int column);
double assess_stone_top_left(int row, int column);
double assess_stone_top_right(int row, int column);
double assess_stone_bottom_left(int row, int column);
double assess_stone_bottom_right(int row, int column);
int end_battle(bool &ref_is_game_running);
int check_winner();
void highlight_winner_vertical(int end_row, int end_column);
void highlight_winner_horizontal(int end_row, int end_column);
void highlight_winner_major_diagonal(int end_row, int end_column);
void highlight_winner_minor_diagonal(int end_row, int end_column);
void show_ending_message(int winner);
int check_next_battle(bool &ref_is_game_running);


int main()
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
        return -1;
    }

    return 0;
}


/*
<Summary> :: sets console properties before the main cycle of the game
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be the line number where the error occurs
*/
int set_up_console()
{
    int error_code;

    if ((error_code = enable_mouse_input()))
        return error_code;

    if ((error_code = adjust_console_size()))
        return error_code;

    if ((error_code = adjust_font_size()))
        return error_code;

    if ((error_code = enable_virtual_terminal_sequences()))
        return error_code;

    // sets the console to display Chinese characters, and exits the current function if the output code page is not changed successfully
    if (!SetConsoleOutputCP(CP_UTF8))
        return __LINE__;

    // changes the console title with a virtual terminal sequence
    std::cout << "\x1B]0;Gomoku\x07";

    // hides the console cursor with a virtual terminal sequence
    std::cout << "\x1B[?25l";

    return 0;
}


/*
<Summary> :: enables mouse events of console input and disables the quick edit mode
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be the line number where the error occurs
*/
int enable_mouse_input()
{
    // gets the console input handle, and exits the current function if the handle is not obtained successfully
    HANDLE console_input_handle {GetStdHandle(STD_INPUT_HANDLE)};
    if (console_input_handle == INVALID_HANDLE_VALUE)
        return __LINE__;

    // sets the input mode to process mouse events without quick edit, and exits the current function if the activation fails
    if (!SetConsoleMode(console_input_handle, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS))
        return __LINE__;

    return 0;
}


/*
<Summary> :: adjusts the screen buffer and window of the console to a proper size
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be the line number where the error occurs
*/
int adjust_console_size()
{
    HANDLE console_output_handle;
    COORD screen_buffer_size;
    SMALL_RECT window_size;

    // gets the console output handle, and exits the current function if the handle is not obtained successfully
    console_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_output_handle == INVALID_HANDLE_VALUE)
        return __LINE__;

    // assigns values to the structures that represent the new screen buffer size and window size
    screen_buffer_size.X = 87;
    screen_buffer_size.Y = 38;

    window_size.Top = 0;
    window_size.Bottom = screen_buffer_size.Y - 1;
    window_size.Left = 0;
    window_size.Right = screen_buffer_size.X - 1;

    // sets the window and screen buffer to the new size, and exits the current function if either adjustment fails
    if (!SetConsoleWindowInfo(console_output_handle, TRUE, &window_size))
        return __LINE__;

    if (!SetConsoleScreenBufferSize(console_output_handle, screen_buffer_size))
        return __LINE__;

    return 0;
}


/*
<Summary> :: adjusts the console font to a proper size
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be the line number where the error occurs
*/
int adjust_font_size()
{
    HANDLE console_output_handle;
    CONSOLE_FONT_INFOEX font_information;

    // gets the console output handle, and exits the current function if the handle is not obtained successfully
    console_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_output_handle == INVALID_HANDLE_VALUE)
        return __LINE__;

    // gets the information about the current font, and exits the current function if the information is not obtained successfully
    font_information.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    if (!GetCurrentConsoleFontEx(console_output_handle, FALSE, &font_information))
        return __LINE__;

    // sets the console font to the new size, and exits the current function if the adjustment fails
    font_information.dwFontSize.Y = 20;
    if (!SetCurrentConsoleFontEx(console_output_handle, FALSE, &font_information))
        return __LINE__;

    return 0;
}


/*
<Summary> :: enables virtual terminal sequences of console output
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be the line number where the error occurs
*/
int enable_virtual_terminal_sequences()
{
    HANDLE console_output_handle;
    DWORD console_output_mode;

    // gets the console output handle, and exits the current function if the handle is not obtained successfully
    console_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_output_handle == INVALID_HANDLE_VALUE)
        return __LINE__;

    // gets the current output mode, and exits the current function if the mode is not obtained successfully
    if (!GetConsoleMode(console_output_handle, &console_output_mode))
        return __LINE__;

    // sets the output mode to process virtual terminal sequences, and exits the current function if the activation fails
    if (!SetConsoleMode(console_output_handle, console_output_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
        return __LINE__;

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

    std::cout << "[Error] The program terminates at the line " << error_code << "!\n\n";
    std::cout << "Press Enter to close the game.....";

    // waits for unbuffered keyboard input without echo, and jumps out of the loop if Enter is pressed
    while (_getch() != 13);

    return;
}


/*
<Summary> :: starts the main cycle of the game
<Parameters> :: none
<Return> :: the return value would be 0 if the game ends normally; otherwise the return value would be the line number where the error occurs
*/
int start_game()
{
    int error_code;
    bool is_game_running;

    if ((error_code = show_title_screen()))
        return error_code;
    clear_console();

    do
    {
        if ((error_code = determine_first_mover()))
            return error_code;
        clear_console();

        if ((error_code = start_battle(is_game_running)))
            return error_code;
        clear_console();
    } while (is_game_running);

    return 0;
}


/*
<Summary> :: displays the title screen in the console and waits for the player to press the left mouse button
<Parameters> :: none
<Return> :: the return value would be 0 if console input events are read successfully; otherwise the return value would be the line number where the error occurs
*/
int show_title_screen()
{
    bool is_mouse_pressed {false};
    std::chrono::steady_clock::time_point animation_start_time {std::chrono::steady_clock::now()};

    display_game_title();

    while (!is_mouse_pressed)
    {
        int error_code;

        display_animated_hint(animation_start_time);

        if ((error_code = check_mouse_press(is_mouse_pressed)))
            return error_code;
    }

    return 0;
}


/*
<Summary> :: displays the game title and a colorful Gomoku board as background in the console
<Parameters> :: none
<Return> :: none
*/
void display_game_title()
{
    std::cout << "\n\n\n\n\n\n\n\n\n\n";

    // displays the game title and a yellow Gomoku board with blue grid using virtual terminal sequences
    std::cout << "                        \x1B[33m______________________________________\x1B[0m\n";
    std::cout << "                       \x1B[33m/ \x1B[34m____________________________________ \x1B[33m\\\x1B[0m\n";
    std::cout << "                      \x1B[33m/ \x1B[34m/      .  .     .       .     .  .   \\ \x1B[33m\\\x1B[0m\n";
    std::cout << "                     \x1B[33m/ \x1B[34m/   .  .  .                  .  .  .   \\ \x1B[33m\\\x1B[0m\n";
    std::cout << "                    \x1B[33m/ \x1B[34m/   .  .  .                    .  .  .   \\ \x1B[33m\\\x1B[0m\n";
    std::cout << "                   \x1B[33m/ \x1B[34m/   .  .  .      \x1B[0m五子棋大戰      \x1B[34m.  .  .   \\ \x1B[33m\\\x1B[0m\n";
    std::cout << "                  \x1B[33m/ \x1B[34m/   .  .  .                        .  .  .   \\ \x1B[33m\\\x1B[0m\n";
    std::cout << "                 \x1B[33m/ \x1B[34m/   .  .  .  .  .  .  .  .  .  .  .  .  .  .   \\ \x1B[33m\\\x1B[0m\n";
    std::cout << "                \x1B[33m/ \x1B[34m/________________________________________________\\ \x1B[33m\\\x1B[0m\n";
    std::cout << "               \x1B[33m/______________________________________________________\\\x1B[0m\n";
    std::cout << "               \x1B[33m|                                                      |\x1B[0m\n";
    std::cout << "               \x1B[33m|______________________________________________________|\x1B[0m\n";

    std::cout << "\n\n\n\n";

    return;
}


/*
<Summary> :: displays or hides the hint text about pressing left mouse button based on how long the last animation has continued
<Parameter "ref_animation_start_time"> :: a reference to the variable storing the time when  the last animation cycle starts
<Return> :: none
*/
void display_animated_hint(std::chrono::steady_clock::time_point &ref_animation_start_time)
{
    std::chrono::steady_clock::time_point current_time {std::chrono::steady_clock::now()};
    std::chrono::milliseconds animation_duration {std::chrono::duration_cast<std::chrono::milliseconds>(current_time - ref_animation_start_time)};

    if (animation_duration.count() <= 750)
        std::cout << "                                 點擊滑鼠左鍵開始遊戲\r";
    else if (animation_duration.count() <= 1500)
        std::cout << "                                                     \r";
    else
        ref_animation_start_time = current_time;

    return;
}


/*
<Summary> :: checks whether the player pressed the left mouse button and stores the information to a specified variable
<Parameter "ref_is_mouse_pressed"> :: a reference to the variable indicating whether the player pressed the left mouse button
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be the line number where the error occurs
*/
int check_mouse_press(bool &ref_is_mouse_pressed)
{
    HANDLE console_input_handle;
    DWORD total_unread_inputs;

    ref_is_mouse_pressed = false;

    // gets the console input handle, and exits the current function if the handle is not obtained successfully
    console_input_handle = GetStdHandle(STD_INPUT_HANDLE);
    if (console_input_handle == INVALID_HANDLE_VALUE)
        return __LINE__;

    // gets the number of unread console input records, and exits the current function if the number is not obtained successfully
    if (!GetNumberOfConsoleInputEvents(console_input_handle, &total_unread_inputs))
        return __LINE__;

    if (total_unread_inputs > 0)
    {
        INPUT_RECORD input_records[1];
        DWORD total_inputs_read;

        // reads one record from the console input buffer and removes it from the buffer, and exits the current function if the input record is not read successfully
        if (!ReadConsoleInput(console_input_handle, input_records, sizeof(input_records) / sizeof(INPUT_RECORD), &total_inputs_read))
            return __LINE__;

        // checks whether the player pressed the left mouse button and stores the information to a specified variable
        if (input_records[0].EventType == MOUSE_EVENT)
            if (input_records[0].Event.MouseEvent.dwEventFlags == 0 && input_records[0].Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
                ref_is_mouse_pressed = true;
    }

    return 0;
}


/*
<Summary> :: clears the console and moves the cursor to the initial position
<Parameters> :: none
<Return> :: none
*/
void clear_console()
{
    // clears the console and moves the cursor to the initial position with virtual terminal sequences
    std::cout << "\x1B[2J\x1B[1;1H";

    return;
}


/*
<Summary> :: displays two cards and asks the player to choose one to randomly determine the first mover of battle
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be the line number where the error occurs
*/
int determine_first_mover()
{
    bool is_card_selected {false};

    initialize_selection_interface();

    while (!is_card_selected)
    {
        int error_code;
        POINT pixel_position_of_mouse;
        bool is_mouse_pressed;

        if ((error_code = get_mouse_position(pixel_position_of_mouse)))
            return error_code;

        refresh_selection_interface(pixel_position_of_mouse);

        if ((error_code = check_mouse_press(is_mouse_pressed)))
            return error_code;

        if (is_mouse_pressed)
            if ((error_code = check_card_selection(pixel_position_of_mouse, is_card_selected)))
                return error_code;
    }

    return 0;
}


/*
<Summary> :: displays the initial selection interface in the console, which includes two cards and the hint text about choosing one
<Parameters> :: none
<Return> :: none
*/
void initialize_selection_interface()
{
    move_cursor(1, 1);

    std::cout << "\n\n\n\n\n\n\n\n";

    // displays two red cards with a yellow cat on the back using virtual terminal sequences
    std::cout << "                   \x1B[31m________________                ________________\x1B[0m\n";
    std::cout << "                  \x1B[31m|                |              |                |\x1B[0m\n";
    std::cout << "                  \x1B[31m|     \x1B[33m|\\___/|    \x1B[31m|              |     \x1B[33m|\\___/|    \x1B[31m|\x1B[0m\n";
    std::cout << "                  \x1B[31m|     \x1B[33m}     {    \x1B[31m|              |     \x1B[33m}     {    \x1B[31m|\x1B[0m\n";
    std::cout << "                  \x1B[31m|     \x1B[33m\\     /    \x1B[31m|              |     \x1B[33m\\     /    \x1B[31m|\x1B[0m\n";
    std::cout << "                  \x1B[31m|      \x1B[33m}***{     \x1B[31m|              |      \x1B[33m}***{     \x1B[31m|\x1B[0m\n";
    std::cout << "                  \x1B[31m|     \x1B[33m/     \\    \x1B[31m|              |     \x1B[33m/     \\    \x1B[31m|\x1B[0m\n";
    std::cout << "                  \x1B[31m|     \x1B[33m|     |    \x1B[31m|              |     \x1B[33m|     |    \x1B[31m|\x1B[0m\n";
    std::cout << "                  \x1B[31m|    \x1B[33m/       \\   \x1B[31m|              |    \x1B[33m/       \\   \x1B[31m|\x1B[0m\n";
    std::cout << "                  \x1B[31m|    \x1B[33m\\       /   \x1B[31m|              |    \x1B[33m\\       /   \x1B[31m|\x1B[0m\n";
    std::cout << "                  \x1B[31m|     \x1B[33m\\__ __/    \x1B[31m|              |     \x1B[33m\\__ __/    \x1B[31m|\x1B[0m\n";
    std::cout << "                  \x1B[31m|       \x1B[33m((       \x1B[31m|              |       \x1B[33m((       \x1B[31m|\x1B[0m\n";
    std::cout << "                  \x1B[31m|       \x1B[33m))       \x1B[31m|              |       \x1B[33m))       \x1B[31m|\x1B[0m\n";
    std::cout << "                  \x1B[31m|________________|              |________________|\x1B[0m\n";

    std::cout << "\n\n";

    std::cout << "                  ==================================================\n";
    std::cout << "                  |                                                |\n";
    std::cout << "                  |          點選其中一張卡片決定落子順序          |\n";
    std::cout << "                  |                                                |\n";
    std::cout << "                  ==================================================\n";

    return;
}


/*
<Summary> :: moves the cursor to a specified position
<Parameter "new_line"> :: the new line where the cursor is located
<Parameter "new_column"> :: the new column where the cursor is located
<Return> :: none
*/
void move_cursor(int new_line, int new_column)
{
    // moves the cursor to the specified line and column with a virtual terminal sequence
    std::cout << "\x1B[" << new_line << ";" << new_column << "H";

    return;
}


/*
<Summary> :: stores the console-area coordinates of the mouse cursor to a specified structure
<Parameter "ref_pixel_position_of_mouse"> :: a reference to the structure storing the console-area coordinates of the mouse cursor
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be the line number where the error occurs
*/
int get_mouse_position(POINT &ref_pixel_position_of_mouse)
{
    // gets the console window handle, and exits the current function if the handle is not obtained successfully
    HWND console_window_handle {GetConsoleWindow()};
    if (console_window_handle == NULL)
        return __LINE__;

    // gets the mouse cursor position in screen coordinates, and exits the current function if the position is not obtained successfully
    if (!GetCursorPos(&ref_pixel_position_of_mouse))
        return __LINE__;

    // converts the screen coordinates of the mouse cursor to console-area coordinates
    if (!ScreenToClient(console_window_handle, &ref_pixel_position_of_mouse))
        return __LINE__;

    return 0;
}


/*
<Summary> :: updates the selection interface based on the mouse cursor position
<Parameter "pixel_position_of_mouse"> :: a structure for storing the console-area coordinates of the mouse cursor
<Return> :: none
*/
void refresh_selection_interface(POINT pixel_position_of_mouse)
{
    if (pixel_position_of_mouse.y >= 179 && pixel_position_of_mouse.y <= 439)
    {
        if (pixel_position_of_mouse.x >= 191 && pixel_position_of_mouse.x <= 369)
            highlight_left_card();
        else if (pixel_position_of_mouse.x >= 524 && pixel_position_of_mouse.x <= 702)
            highlight_right_card();
        else
            initialize_selection_interface();
    }
    else
    {
        initialize_selection_interface();
    }

    return;
}


/*
<Summary> :: highlights the left card using brighter color
<Parameters> :: none
<Return> :: none
*/
void highlight_left_card()
{
    // highlights the left card as bright red one with a bright yellow cat on the back using virtual terminal sequences
    move_cursor(9, 19);
    std::cout << " \x1B[91m________________\x1B[0m";
    move_cursor(10, 19);
    std::cout << "\x1B[91m|                |\x1B[0m";
    move_cursor(11, 19);
    std::cout << "\x1B[91m|     \x1B[93m|\\___/|    \x1B[91m|\x1B[0m";
    move_cursor(12, 19);
    std::cout << "\x1B[91m|     \x1B[93m}     {    \x1B[91m|\x1B[0m";
    move_cursor(13, 19);
    std::cout << "\x1B[91m|     \x1B[93m\\     /    \x1B[91m|\x1B[0m";
    move_cursor(14, 19);
    std::cout << "\x1B[91m|      \x1B[93m}***{     \x1B[91m|\x1B[0m";
    move_cursor(15, 19);
    std::cout << "\x1B[91m|     \x1B[93m/     \\    \x1B[91m|\x1B[0m";
    move_cursor(16, 19);
    std::cout << "\x1B[91m|     \x1B[93m|     |    \x1B[91m|\x1B[0m";
    move_cursor(17, 19);
    std::cout << "\x1B[91m|    \x1B[93m/       \\   \x1B[91m|\x1B[0m";
    move_cursor(18, 19);
    std::cout << "\x1B[91m|    \x1B[93m\\       /   \x1B[91m|\x1B[0m";
    move_cursor(19, 19);
    std::cout << "\x1B[91m|     \x1B[93m\\__ __/    \x1B[91m|\x1B[0m";
    move_cursor(20, 19);
    std::cout << "\x1B[91m|       \x1B[93m((       \x1B[91m|\x1B[0m";
    move_cursor(21, 19);
    std::cout << "\x1B[91m|       \x1B[93m))       \x1B[91m|\x1B[0m";
    move_cursor(22, 19);
    std::cout << "\x1B[91m|________________|\x1B[0m";

    return;
}


/*
<Summary> :: highlights the right card using brighter color
<Parameters> :: none
<Return> :: none
*/
void highlight_right_card()
{
    // highlights the right card as bright red one with a bright yellow cat on the back using virtual terminal sequences
    move_cursor(9, 51);
    std::cout << " \x1B[91m________________\x1B[0m";
    move_cursor(10, 51);
    std::cout << "\x1B[91m|                |\x1B[0m";
    move_cursor(11, 51);
    std::cout << "\x1B[91m|     \x1B[93m|\\___/|    \x1B[91m|\x1B[0m";
    move_cursor(12, 51);
    std::cout << "\x1B[91m|     \x1B[93m}     {    \x1B[91m|\x1B[0m";
    move_cursor(13, 51);
    std::cout << "\x1B[91m|     \x1B[93m\\     /    \x1B[91m|\x1B[0m";
    move_cursor(14, 51);
    std::cout << "\x1B[91m|      \x1B[93m}***{     \x1B[91m|\x1B[0m";
    move_cursor(15, 51);
    std::cout << "\x1B[91m|     \x1B[93m/     \\    \x1B[91m|\x1B[0m";
    move_cursor(16, 51);
    std::cout << "\x1B[91m|     \x1B[93m|     |    \x1B[91m|\x1B[0m";
    move_cursor(17, 51);
    std::cout << "\x1B[91m|    \x1B[93m/       \\   \x1B[91m|\x1B[0m";
    move_cursor(18, 51);
    std::cout << "\x1B[91m|    \x1B[93m\\       /   \x1B[91m|\x1B[0m";
    move_cursor(19, 51);
    std::cout << "\x1B[91m|     \x1B[93m\\__ __/    \x1B[91m|\x1B[0m";
    move_cursor(20, 51);
    std::cout << "\x1B[91m|       \x1B[93m((       \x1B[91m|\x1B[0m";
    move_cursor(21, 51);
    std::cout << "\x1B[91m|       \x1B[93m))       \x1B[91m|\x1B[0m";
    move_cursor(22, 51);
    std::cout << "\x1B[91m|________________|\x1B[0m";

    return;
}


/*
<Summary> :: plays the flip animation and sound effects if the player chooses a card, and stores the selection information to specified variables
<Parameter "pixel_position_of_mouse"> :: a structure for storing the console-area coordinates of the mouse cursor
<Parameter "ref_is_card_selected"> :: a reference to the variable indicating whether one of the cards is selected
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be the line number where the error occurs
*/
int check_card_selection(POINT pixel_position_of_mouse, bool &ref_is_card_selected)
{
    if (pixel_position_of_mouse.y >= 179 && pixel_position_of_mouse.y <= 439)
    {
        if (pixel_position_of_mouse.x >= 191 && pixel_position_of_mouse.x <= 369)
        {
            ref_is_card_selected = true;

            std::srand(std::time(nullptr));
            is_player_turn = std::rand() % 2;

            // plays asynchronous sound effects when a card is selected, and exits the current function if the sound effects are not played successfully
            if (!PlaySound(TEXT("choosing_card.wav"), NULL, SND_FILENAME | SND_ASYNC))
                return __LINE__;

            turn_over_left_card();
        }
        else if (pixel_position_of_mouse.x >= 524 && pixel_position_of_mouse.x <= 702)
        {
            ref_is_card_selected = true;

            std::srand(std::time(nullptr));
            is_player_turn = std::rand() % 2;

            // plays asynchronous sound effects when a card is selected, and exits the current function if the sound effects are not played successfully
            if (!PlaySound(TEXT("choosing_card.wav"), NULL, SND_FILENAME | SND_ASYNC))
                return __LINE__;

            turn_over_right_card();
        }
    }

    return 0;
}


/*
<Summary> :: turns over the left card and displays the player's order of moves
<Parameters> :: none
<Return> :: none
*/
void turn_over_left_card()
{
    move_cursor(11, 20);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(12, 20);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(13, 20);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(14, 20);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(15, 20);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(16, 20);
    if (is_player_turn)
        std::cout << "      先手      ";
    else
        std::cout << "      後手      ";    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(17, 20);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(18, 20);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(19, 20);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(20, 20);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(21, 20);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(750));

    return;
}


/*
<Summary> :: turns over the right card and displays the player's order of moves
<Parameters> :: none
<Return> :: none
*/
void turn_over_right_card()
{
    move_cursor(11, 52);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(12, 52);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(13, 52);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(14, 52);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(15, 52);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(16, 52);
    if (is_player_turn)
        std::cout << "      先手      ";
    else
        std::cout << "      後手      ";    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(17, 52);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(18, 52);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(19, 52);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(20, 52);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    move_cursor(21, 52);
    std::cout << "                ";
    std::this_thread::sleep_for(std::chrono::milliseconds(750));

    return;
}


/*
<Summary> :: starts a new cycle of battle, and checks whether the player wants to play again at the end
<Parameter "ref_is_game_running"> :: a reference to the variable indicating whether the player wants to play again
<Return> :: the return value would be 0 if the battle ends normally; otherwise the return value would be the line number where the error occurs
*/
int start_battle(bool &ref_is_game_running)
{
    int error_code;

    initialize_battle();

    do
    {
        if (is_player_turn)
        {
            if ((error_code = read_player_move()))
                return error_code;
            is_player_turn = false;
        }
        else
        {
            if ((error_code = perform_ai_move()))
                return error_code;
            is_player_turn = true;
        }
    } while (!check_battle_state());

    if ((error_code = end_battle(ref_is_game_running)))
        return error_code;

    return 0;
}


/*
<Summary> :: initializes the Gomoku board, last move record, and battle interface
<Parameters> :: none
<Return> :: none
*/
void initialize_battle()
{
    for (int row {0}; row < 15; row++)
        for (int column {0}; column < 15; column++)
            gomoku_board[row][column] = 0;

    last_placed_row = -1;
    last_placed_column = -1;

    std::cout << "\n\n";

    // displays blue Gomoku board grid with a virtual terminal sequence
    std::cout << "               \x1B[34m+---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n";
    std::cout << "               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |\n";
    std::cout << "               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n";
    std::cout << "               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |\n";
    std::cout << "               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n";
    std::cout << "               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |\n";
    std::cout << "               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n";
    std::cout << "               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |\n";
    std::cout << "               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n";
    std::cout << "               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |\n";
    std::cout << "               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n";
    std::cout << "               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |\n";
    std::cout << "               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n";
    std::cout << "               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |\n";
    std::cout << "               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n";
    std::cout << "               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |\n";
    std::cout << "               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n";
    std::cout << "               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |\n";
    std::cout << "               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n";
    std::cout << "               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |\n";
    std::cout << "               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n";
    std::cout << "               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |\n";
    std::cout << "               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n";
    std::cout << "               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |\n";
    std::cout << "               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n";
    std::cout << "               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |\n";
    std::cout << "               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n";
    std::cout << "               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |\n";
    std::cout << "               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+\x1B[0m\n";
    std::cout << "               =========================================================\n";
    std::cout << "               |                                                       |\n";
    std::cout << "               |                                                       |\n";
    std::cout << "               |                                                       |\n";
    std::cout << "               =========================================================\n";

    return;
}


/*
<Summary> :: reads the player's next move, and then updates the battle interface and plays sound effects accordingly
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be the line number where the error occurs
*/
int read_player_move()
{
    int error_code;
    COORD character_position_of_click;
    int placed_row;
    int placed_column;

    move_cursor(34, 17);
    std::cout << "   輪到你的回合，在棋盤上的空位點擊滑鼠左鍵放置棋子    ";

    if ((error_code = read_stone_placement(character_position_of_click, placed_row, placed_column)))
        return error_code;

    // plays asynchronous sound effects when a stone is placed, and exits the current function if the sound effects are not played successfully
    if (!PlaySound(TEXT("placing_stone.wav"), NULL, SND_FILENAME | SND_ASYNC))
        return __LINE__;

    refresh_gomoku_board(character_position_of_click);

    gomoku_board[placed_row][placed_column] = 1;
    last_placed_row = placed_row;
    last_placed_column = placed_column;

    return 0;
}


/*
<Summary> :: waits until the player places a stone, and stores the placement information to specified variables
<Parameter "ref_character_position_of_click"> :: a reference to the structure storing the character coordinates of the placed stone
<Parameter "ref_placed_row"> :: a reference to the variable storing the row where the player places the stone
<Parameter "ref_placed_column"> :: a reference to the variable storing the column where the player places the stone
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be the line number where the error occurs
*/
int read_stone_placement(COORD &ref_character_position_of_click, int &ref_placed_row, int &ref_placed_column)
{
    HANDLE console_input_handle;
    bool is_stone_placed;

    // gets the console input handle, and exits the current function if the handle is not obtained successfully
    console_input_handle = GetStdHandle(STD_INPUT_HANDLE);
    if (console_input_handle == INVALID_HANDLE_VALUE)
        return __LINE__;

    // flushes the console input buffer, and exits the current function if the input records are not discarded successfully
    if (!FlushConsoleInputBuffer(console_input_handle))
        return __LINE__;

    is_stone_placed = false;
    while (!is_stone_placed)
    {
        INPUT_RECORD input_records[1];
        DWORD total_inputs_read;

        // reads one record from the console input buffer and removes it from the buffer, and exits the current function if the input record is not read successfully
        if (!ReadConsoleInput(console_input_handle, input_records, sizeof(input_records) / sizeof(INPUT_RECORD), &total_inputs_read))
            return __LINE__;

        is_stone_placed = check_valid_placement(input_records[0], ref_character_position_of_click, ref_placed_row, ref_placed_column);
    }

    return 0;
}


/*
<Summary> :: stores the placement information to specified variables if the player clicks an empty point to place a stone
<Parameter "input_record"> :: a structure for storing an input event read from the console input buffer
<Parameter "ref_character_position_of_click"> :: a reference to the structure storing the character coordinates of the placed stone
<Parameter "ref_placed_row"> :: a reference to the variable storing the row where the player places the stone
<Parameter "ref_placed_column"> :: a reference to the variable storing the column where the player places the stone
<Return> :: whether the player has placed a stone
*/
bool check_valid_placement(INPUT_RECORD input_record, COORD &ref_character_position_of_click, int &ref_placed_row, int &ref_placed_column)
{
    bool is_stone_placed {false};

    if (input_record.EventType == MOUSE_EVENT)
    {
        MOUSE_EVENT_RECORD mouse_event {input_record.Event.MouseEvent};

        if (mouse_event.dwEventFlags == 0 && mouse_event.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
        {
            if (mouse_event.dwMousePosition.X >= 15 && mouse_event.dwMousePosition.X <= 71 && mouse_event.dwMousePosition.Y >= 2 && mouse_event.dwMousePosition.Y <= 35)
            {
                if ((mouse_event.dwMousePosition.X - 15) % 4 == 0 && (mouse_event.dwMousePosition.Y - 2) % 2 == 0)
                {
                    int clicked_row {(mouse_event.dwMousePosition.Y - 2) / 2};
                    int clicked_column {(mouse_event.dwMousePosition.X - 15) / 4};

                    if (gomoku_board[clicked_row][clicked_column] == 0)
                    {
                        ref_character_position_of_click = mouse_event.dwMousePosition;
                        ref_placed_row = clicked_row;
                        ref_placed_column = clicked_column;
                        is_stone_placed = true;
                    }
                }
            }
        }
    }

    return is_stone_placed;
}


/*
<Summary> :: updates the new move of the player or AI to the Gomoku board interface
<Parameter "character_position_of_click"> :: a structure for storing the character coordinates of the placed stone
<Return> :: none
*/
void refresh_gomoku_board(COORD character_position_of_click)
{
    if (is_player_turn)
    {
        move_cursor(character_position_of_click.Y + 1, character_position_of_click.X + 1);
        std::cout << "O";

        if (last_placed_row != -1 || last_placed_column != -1)
        {
            COORD last_placed_character_position {
                static_cast<SHORT>(last_placed_column * 4 + 15),
                static_cast<SHORT>(last_placed_row * 2 + 2)
            };

            move_cursor(last_placed_character_position.Y + 1, last_placed_character_position.X + 1);

            // displays a red stone with a virtual terminal sequence to unhighlight the AI's last move
            std::cout << "\x1B[31mO\x1B[0m";
        }
    }
    else
    {
        move_cursor(character_position_of_click.Y + 1, character_position_of_click.X + 1);

        // displays a bright red stone with a virtual terminal sequence
        std::cout << "\x1B[91mO\x1B[0m";
    }

    return;
}


/*
<Summary> :: calculates the AI's next move, and then updates the battle interface and plays sound effects accordingly
<Parameters> :: none
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be the line number where the error occurs
*/
int perform_ai_move()
{
    COORD character_position_of_click;
    int placed_row;
    int placed_column;

    move_cursor(34, 17);
    std::cout << "          輪到對手的回合，等待他完成下一步棋           ";

    std::this_thread::sleep_for(std::chrono::seconds(1));

    calculate_ai_move(character_position_of_click, placed_row, placed_column);

    // plays asynchronous sound effects when a stone is placed, and exits the current function if the sound effects are not played successfully
    if (!PlaySound(TEXT("placing_stone.wav"), NULL, SND_FILENAME | SND_ASYNC))
        return __LINE__;

    refresh_gomoku_board(character_position_of_click);

    gomoku_board[placed_row][placed_column] = -1;
    last_placed_row = placed_row;
    last_placed_column = placed_column;

    return 0;
}


/*
<Summary> :: calculates the AI's next move using the minimax algorithm with alpha-beta pruning, and stores the placement information to specified variables
<Parameter "ref_character_position_of_click"> :: a reference to the structure storing the character coordinates of the placed stone
<Parameter "ref_placed_row"> :: a reference to the variable storing the row where the AI places the stone
<Parameter "ref_placed_column"> :: a reference to the variable storing the column where the AI places the stone
<Return> :: none
*/
void calculate_ai_move(COORD &ref_character_position_of_click, int &ref_placed_row, int &ref_placed_column)
{
    if (last_placed_row == -1 && last_placed_column == -1)
    {
        ref_placed_row = 7;
        ref_placed_column = 7;
        ref_character_position_of_click.X = ref_placed_column * 4 + 15;
        ref_character_position_of_click.Y = ref_placed_row * 2 + 2;
    }
    else
    {
        double max_board_value {std::numeric_limits<double>::lowest()};
        double min_board_value {std::numeric_limits<double>::max()};

        for (int row {0}; row < 15; row++)
        {
            for (int column {0}; column < 15; column++)
            {
                if (gomoku_board[row][column] == 0 && check_neighbors(row, column))
                {
                    int temp_row;
                    int temp_column;
                    double board_value;

                    gomoku_board[row][column] = -1;
                    temp_row = last_placed_row;
                    temp_column = last_placed_column;
                    last_placed_row = row;
                    last_placed_column = column;

                    board_value = predict_board_value(true, 2, max_board_value, min_board_value);

                    gomoku_board[row][column] = 0;
                    last_placed_row = temp_row;
                    last_placed_column = temp_column;

                    if (board_value > max_board_value)
                    {
                        max_board_value = board_value;
                        ref_placed_row = row;
                        ref_placed_column = column;
                    }
                }
            }
        }

        ref_character_position_of_click.X = ref_placed_column * 4 + 15;
        ref_character_position_of_click.Y = ref_placed_row * 2 + 2;
    }

    return;
}


/*
<Summary> :: checks whether a specified position of the gomoku board has any adjacent stones
<Parameter "row"> :: a row index of the gomoku board
<Parameter "column"> :: a column index of the gomoku board
<Return> :: whether the specified position of the gomoku board has any adjacent stones
*/
bool check_neighbors(int row, int column)
{
    if (row - 1 >= 0 && column - 1 >= 0)
        if (gomoku_board[row - 1][column - 1] != 0)
            return true;

    if (row - 1 >= 0)
        if (gomoku_board[row - 1][column] != 0)
            return true;

    if (row - 1 >= 0 && column + 1 <= 14)
        if (gomoku_board[row - 1][column + 1] != 0)
            return true;

    if (column - 1 >= 0)
        if (gomoku_board[row][column - 1] != 0)
            return true;

    if (column + 1 <= 14)
        if (gomoku_board[row][column + 1] != 0)
            return true;

    if (row + 1 <= 14 && column - 1 >= 0)
        if (gomoku_board[row + 1][column - 1] != 0)
            return true;

    if (row + 1 <= 14)
        if (gomoku_board[row + 1][column] != 0)
            return true;

    if (row + 1 <= 14 && column + 1 <= 14)
        if (gomoku_board[row + 1][column + 1] != 0)
            return true;

    return false;
}


/*
<Summary> :: predicts the board value using the minimax algorithm with alpha-beta pruning
<Parameter "is_player_next"> :: whether the player moves next
<Parameter "search_depth"> :: the number of moves to be predicted
<Parameter "max_board_value"> :: the maximum board value that the AI has found
<Parameter "min_board_value"> :: the minimum board value that the player has found
<Return> :: the predicted board value
*/
double predict_board_value(bool is_player_next, int search_depth, double max_board_value, double min_board_value)
{
    // returns the current board value if a leaf node of recursion tree is found
    if (search_depth == 0 || check_battle_state())
        return assess_board_value();

    // calculates and returns the minimum board value for the player turn
    if (is_player_next)
    {
        for (int row {0}; row < 15; row++)
        {
            for (int column {0}; column < 15; column++)
            {
                if (gomoku_board[row][column] == 0 && check_neighbors(row, column))
                {
                    int temp_row;
                    int temp_column;
                    double board_value;

                    gomoku_board[row][column] = 1;
                    temp_row = last_placed_row;
                    temp_column = last_placed_column;
                    last_placed_row = row;
                    last_placed_column = column;

                    board_value = predict_board_value(!is_player_next, search_depth - 1, max_board_value, min_board_value);

                    gomoku_board[row][column] = 0;
                    last_placed_row = temp_row;
                    last_placed_column = temp_column;

                    if (board_value < min_board_value)
                        min_board_value = board_value;

                    if (min_board_value <= max_board_value)
                        return min_board_value;
                }
            }
        }

        return min_board_value;
    }
    // calculates and returns the maximum board value for the AI turn
    else
    {
        for (int row {0}; row < 15; row++)
        {
            for (int column {0}; column < 15; column++)
            {
                if (gomoku_board[row][column] == 0 && check_neighbors(row, column))
                {
                    int temp_row;
                    int temp_column;
                    double board_value;

                    gomoku_board[row][column] = -1;
                    temp_row = last_placed_row;
                    temp_column = last_placed_column;
                    last_placed_row = row;
                    last_placed_column = column;

                    board_value = predict_board_value(!is_player_next, search_depth - 1, max_board_value, min_board_value);

                    gomoku_board[row][column] = 0;
                    last_placed_row = temp_row;
                    last_placed_column = temp_column;

                    if (board_value > max_board_value)
                        max_board_value = board_value;

                    if (min_board_value <= max_board_value)
                        return max_board_value;
                }
            }
        }

        return max_board_value;
    }
}


/*
<Summary> :: checks whether the player of the last move wins or the battle ends in a tie
<Parameters> :: none
<Return> :: whether the battle is over
*/
bool check_battle_state()
{
    int last_moved_player {gomoku_board[last_placed_row][last_placed_column]};

    // checks whether the player of the last move forms an unbroken line of five stones vertically
    for (int total_adjacent_stones {0}, offset {-4}; offset < 5; offset++)
    {
        if (last_placed_row + offset >= 0 && last_placed_row + offset <= 14)
        {
            if (gomoku_board[last_placed_row + offset][last_placed_column] == last_moved_player)
                total_adjacent_stones++;
            else
                total_adjacent_stones = 0;
        }

        if (total_adjacent_stones == 5)
            return true;
    }

    // checks whether the player of the last move forms an unbroken line of five stones horizontally
    for (int total_adjacent_stones {0}, offset {-4}; offset < 5; offset++)
    {
        if (last_placed_column + offset >= 0 && last_placed_column + offset <= 14)
        {
            if (gomoku_board[last_placed_row][last_placed_column + offset] == last_moved_player)
                total_adjacent_stones++;
            else
                total_adjacent_stones = 0;
        }

        if (total_adjacent_stones == 5)
            return true;
    }

    // checks whether the player of the last move forms an unbroken diagonal line of five stones from top left to bottom right
    for (int total_adjacent_stones {0}, offset {-4}; offset < 5; offset++)
    {
        if (last_placed_row + offset >= 0 && last_placed_row + offset <= 14 && last_placed_column + offset >= 0 && last_placed_column + offset <= 14)
        {
            if (gomoku_board[last_placed_row + offset][last_placed_column + offset] == last_moved_player)
                total_adjacent_stones++;
            else
                total_adjacent_stones = 0;
        }

        if (total_adjacent_stones == 5)
            return true;
    }

    // checks whether the player of the last move forms an unbroken diagonal line of five stones from bottom left to top right
    for (int total_adjacent_stones {0}, offset {-4}; offset < 5; offset++)
    {
        if (last_placed_row - offset >= 0 && last_placed_row - offset <= 14 && last_placed_column + offset >= 0 && last_placed_column + offset <= 14)
        {
            if (gomoku_board[last_placed_row - offset][last_placed_column + offset] == last_moved_player)
                total_adjacent_stones++;
            else
                total_adjacent_stones = 0;
        }

        if (total_adjacent_stones == 5)
            return true;
    }

    // checks whether the gomoku board is completely filled with stones
    for (int row {0}; row < 15; row++)
        for (int column {0}; column < 15; column++)
            if (gomoku_board[row][column] == 0)
                return false;
    return true;
}


/*
<Summary> :: assesses the total value of the current board state
<Parameters> :: none
<Return> :: the total value of the current board state
*/
double assess_board_value()
{
    double board_value {0.0};

    for (int row {0}; row < 15; row++)
    {
        for (int column {0}; column < 15; column++)
        {
            if (gomoku_board[row][column] != 0)
            {
                double stone_value {0.0};

                stone_value += assess_stone_top(row, column);
                stone_value += assess_stone_bottom(row, column);
                stone_value += assess_stone_left(row, column);
                stone_value += assess_stone_right(row, column);
                stone_value += assess_stone_top_left(row, column);
                stone_value += assess_stone_top_right(row, column);
                stone_value += assess_stone_bottom_left(row, column);
                stone_value += assess_stone_bottom_right(row, column);
                stone_value += 0.1 * (15 - abs(row - 7) - abs(column - 7));

                if (gomoku_board[row][column] == 1)
                    // sets the AI to focus more on defense
                    stone_value = -stone_value * 5;

                board_value += stone_value;
            }
        }
    }

    return board_value;
}


/*
<Summary> :: assesses the value related to the top direction of a specified stone
<Parameter "row"> :: the row index of a stone
<Parameter "column"> :: the column index of a stone
<Return> :: the value related to the top direction of the specified stone
*/
double assess_stone_top(int row, int column)
{
    double top_value {10.0};
    int total_extendable_points {0};

    for (int r {row - 1}; r >= 0 && gomoku_board[r][column] != -gomoku_board[row][column] && total_extendable_points < 4; total_extendable_points++, r--);

    if (total_extendable_points < 4)
        top_value = 0.0;
    else
        for (int r {row - 1}; gomoku_board[r][column] == gomoku_board[row][column]; top_value *= 10.0, r--);

    return top_value;
}


/*
<Summary> :: assesses the value related to the bottom direction of a specified stone
<Parameter "row"> :: the row index of a stone
<Parameter "column"> :: the column index of a stone
<Return> :: the value related to the bottom direction of the specified stone
*/
double assess_stone_bottom(int row, int column)
{
    double bottom_value {10.0};
    int total_extendable_points {0};

    for (int r {row + 1}; r <= 14 && gomoku_board[r][column] != -gomoku_board[row][column] && total_extendable_points < 4; total_extendable_points++, r++);

    if (total_extendable_points < 4)
        bottom_value = 0.0;
    else
        for (int r {row + 1}; gomoku_board[r][column] == gomoku_board[row][column]; bottom_value *= 10.0, r++);

    return bottom_value;
}


/*
<Summary> :: assesses the value related to the left direction of a specified stone
<Parameter "row"> :: the row index of a stone
<Parameter "column"> :: the column index of a stone
<Return> :: the value related to the left direction of the specified stone
*/
double assess_stone_left(int row, int column)
{
    double left_value {10.0};
    int total_extendable_points {0};

    for (int c {column - 1}; c >= 0 && gomoku_board[row][c] != -gomoku_board[row][column] && total_extendable_points < 4; total_extendable_points++, c--);

    if (total_extendable_points < 4)
        left_value = 0.0;
    else
        for (int c {column - 1}; gomoku_board[row][c] == gomoku_board[row][column]; left_value *= 10.0, c--);

    return left_value;
}


/*
<Summary> :: assesses the value related to the right direction of a specified stone
<Parameter "row"> :: the row index of a stone
<Parameter "column"> :: the column index of a stone
<Return> :: the value related to the right direction of the specified stone
*/
double assess_stone_right(int row, int column)
{
    double right_value {10.0};
    int total_extendable_points {0};

    for (int c {column + 1}; c <= 14 && gomoku_board[row][c] != -gomoku_board[row][column] && total_extendable_points < 4; total_extendable_points++, c++);

    if (total_extendable_points < 4)
        right_value = 0.0;
    else
        for (int c {column + 1}; gomoku_board[row][c] == gomoku_board[row][column]; right_value *= 10.0, c++);

    return right_value;
}


/*
<Summary> :: assesses the value related to the top left direction of a specified stone
<Parameter "row"> :: the row index of a stone
<Parameter "column"> :: the column index of a stone
<Return> :: the value related to the top left direction of the specified stone
*/
double assess_stone_top_left(int row, int column)
{
    double top_left_value {10.0};
    int total_extendable_points {0};

    for (int r {row - 1}, c {column - 1}; r >= 0 && c >= 0 && gomoku_board[r][c] != -gomoku_board[row][column] && total_extendable_points < 4; total_extendable_points++, r--, c--);

    if (total_extendable_points < 4)
        top_left_value = 0.0;
    else
        for (int r {row - 1}, c {column - 1}; gomoku_board[r][c] == gomoku_board[row][column]; top_left_value *= 10.0, r--, c--);

    return top_left_value;
}


/*
<Summary> :: assesses the value related to the top right direction of a specified stone
<Parameter "row"> :: the row index of a stone
<Parameter "column"> :: the column index of a stone
<Return> :: the value related to the top right direction of the specified stone
*/
double assess_stone_top_right(int row, int column)
{
    double top_right_value {10.0};
    int total_extendable_points {0};

    for (int r {row - 1}, c {column + 1}; r >= 0 && c <= 14 && gomoku_board[r][c] != -gomoku_board[row][column] && total_extendable_points < 4; total_extendable_points++, r--, c++);

    if (total_extendable_points < 4)
        top_right_value = 0.0;
    else
        for (int r {row - 1}, c {column + 1}; gomoku_board[r][c] == gomoku_board[row][column]; top_right_value *= 10.0, r--, c++);

    return top_right_value;
}


/*
<Summary> :: assesses the value related to the bottom left direction of a specified stone
<Parameter "row"> :: the row index of a stone
<Parameter "column"> :: the column index of a stone
<Return> :: the value related to the bottom left direction of the specified stone
*/
double assess_stone_bottom_left(int row, int column)
{
    double bottom_left_value {10.0};
    int total_extendable_points {0};

    for (int r {row + 1}, c {column - 1}; r <= 14 && c >= 0 && gomoku_board[r][c] != -gomoku_board[row][column] && total_extendable_points < 4; total_extendable_points++, r++, c--);

    if (total_extendable_points < 4)
        bottom_left_value = 0.0;
    else
        for (int r {row + 1}, c {column - 1}; gomoku_board[r][c] == gomoku_board[row][column]; bottom_left_value *= 10.0, r++, c--);

    return bottom_left_value;
}


/*
<Summary> :: assesses the value related to the bottom right direction of a specified stone
<Parameter "row"> :: the row index of a stone
<Parameter "column"> :: the column index of a stone
<Return> :: the value related to the bottom right direction of the specified stone
*/
double assess_stone_bottom_right(int row, int column)
{
    double bottom_right_value {10.0};
    int total_extendable_points {0};

    for (int r {row + 1}, c {column + 1}; r <= 14 && c <= 14 && gomoku_board[r][c] != -gomoku_board[row][column] && total_extendable_points < 4; total_extendable_points++, r++, c++);

    if (total_extendable_points < 4)
        bottom_right_value = 0.0;
    else
        for (int r {row + 1}, c {column + 1}; gomoku_board[r][c] == gomoku_board[row][column]; bottom_right_value *= 10.0, r++, c++);

    return bottom_right_value;
}


/*
<Summary> :: displays an ending message based on the battle result, and checks whether the player wants to play again
<Parameter "ref_is_game_running"> :: a reference to the variable indicating whether the player wants to play again
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be the line number where the error occurs
*/
int end_battle(bool &ref_is_game_running)
{
    int error_code;
    int winner;

    winner = check_winner();

    show_ending_message(winner);

    if ((error_code = check_next_battle(ref_is_game_running)))
        return error_code;

    return 0;
}


/*
<Summary> :: checks the battle result and highlights the winning line if the battle does not end in a tie
<Parameters> :: none
<Return> :: the battle result (1 = player's victory, -1 = AI's victory, 0 = tie)
*/
int check_winner()
{
    int last_moved_player {gomoku_board[last_placed_row][last_placed_column]};

    // highlights the line of stones if the player of the last move forms an unbroken line of five stones vertically, and returns the winner
    for (int total_adjacent_stones {0}, offset {-4}; offset < 5; offset++)
    {
        if (last_placed_row + offset >= 0 && last_placed_row + offset <= 14)
        {
            if (gomoku_board[last_placed_row + offset][last_placed_column] == last_moved_player)
                total_adjacent_stones++;
            else
                total_adjacent_stones = 0;
        }

        if (total_adjacent_stones == 5)
        {
            highlight_winner_vertical(last_placed_row + offset, last_placed_column);
            return last_moved_player;
        }
    }

    // highlights the line of stones if the player of the last move forms an unbroken line of five stones horizontally, and returns the winner
    for (int total_adjacent_stones {0}, offset {-4}; offset < 5; offset++)
    {
        if (last_placed_column + offset >= 0 && last_placed_column + offset <= 14)
        {
            if (gomoku_board[last_placed_row][last_placed_column + offset] == last_moved_player)
                total_adjacent_stones++;
            else
                total_adjacent_stones = 0;
        }

        if (total_adjacent_stones == 5)
        {
            highlight_winner_horizontal(last_placed_row, last_placed_column + offset);
            return last_moved_player;
        }
    }

    // highlights the line of stones if the player of the last move forms an unbroken diagonal line of five stones from top left to bottom right, and returns the winner
    for (int total_adjacent_stones {0}, offset {-4}; offset < 5; offset++)
    {
        if (last_placed_row + offset >= 0 && last_placed_row + offset <= 14 && last_placed_column + offset >= 0 && last_placed_column + offset <= 14)
        {
            if (gomoku_board[last_placed_row + offset][last_placed_column + offset] == last_moved_player)
                total_adjacent_stones++;
            else
                total_adjacent_stones = 0;
        }

        if (total_adjacent_stones == 5)
        {
            highlight_winner_major_diagonal(last_placed_row + offset, last_placed_column + offset);
            return last_moved_player;
        }
    }

    // highlights the line of stones if the player of the last move forms an unbroken diagonal line of five stones from bottom left to top right, and returns the winner
    for (int total_adjacent_stones {0}, offset {-4}; offset < 5; offset++)
    {
        if (last_placed_row - offset >= 0 && last_placed_row - offset <= 14 && last_placed_column + offset >= 0 && last_placed_column + offset <= 14)
        {
            if (gomoku_board[last_placed_row - offset][last_placed_column + offset] == last_moved_player)
                total_adjacent_stones++;
            else
                total_adjacent_stones = 0;
        }

        if (total_adjacent_stones == 5)
        {
            highlight_winner_minor_diagonal(last_placed_row - offset, last_placed_column + offset);
            return last_moved_player;
        }
    }

    return 0;
}


/*
<Summary> :: highlights the vertical winning line using brighter and different stones
<Parameter "end_row"> :: the row index of the bottom end of the vertical winning line
<Parameter "end_column"> :: the column index of the bottom end of the vertical winning line
<Return> :: none
*/
void highlight_winner_vertical(int end_row, int end_column)
{
    for (int offset {0}; offset > -5; offset--)
    {
        int line_of_stone = (end_row + offset) * 2 + 3;
        int column_of_stone = end_column * 4 + 16;

        move_cursor(line_of_stone, column_of_stone);

        // displays a brighter and different stone with a virtual terminal sequence to highlight the winning line
        if (gomoku_board[end_row][end_column] == 1)
            std::cout << "\x1B[97m@\x1B[0m";
        else
            std::cout << "\x1B[91m@\x1B[0m";
    }

    return;
}


/*
<Summary> :: highlights the horizontal winning line using brighter and different stones
<Parameter "end_row"> :: the row index of the right end of the horizontal winning line
<Parameter "end_column"> :: the column index of the right end of the horizontal winning line
<Return> :: none
*/
void highlight_winner_horizontal(int end_row, int end_column)
{
    for (int offset {0}; offset > -5; offset--)
    {
        int line_of_stone = end_row * 2 + 3;
        int column_of_stone = (end_column + offset) * 4 + 16;

        move_cursor(line_of_stone, column_of_stone);

        // displays a brighter and different stone with a virtual terminal sequence to highlight the winning line
        if (gomoku_board[end_row][end_column] == 1)
            std::cout << "\x1B[97m@\x1B[0m";
        else
            std::cout << "\x1B[91m@\x1B[0m";
    }

    return;
}


/*
<Summary> :: highlights the diagonal winning line from top left to bottom right using brighter and different stones
<Parameter "end_row"> :: the row index of the bottom right end of the diagonal winning line
<Parameter "end_column"> :: the column index of the bottom right end of the diagonal winning line
<Return> :: none
*/
void highlight_winner_major_diagonal(int end_row, int end_column)
{
    for (int offset {0}; offset > -5; offset--)
    {
        int line_of_stone = (end_row + offset) * 2 + 3;
        int column_of_stone = (end_column + offset) * 4 + 16;

        move_cursor(line_of_stone, column_of_stone);

        // displays a brighter and different stone with a virtual terminal sequence to highlight the winning line
        if (gomoku_board[end_row][end_column] == 1)
            std::cout << "\x1B[97m@\x1B[0m";
        else
            std::cout << "\x1B[91m@\x1B[0m";
    }

    return;
}


/*
<Summary> :: highlights the diagonal winning line from bottom left to top right using brighter and different stones
<Parameter "end_row"> :: the row index of the top right end of the diagonal winning line
<Parameter "end_column"> :: the column index of the top right end of the diagonal winning line
<Return> :: none
*/
void highlight_winner_minor_diagonal(int end_row, int end_column)
{
    for (int offset {0}; offset > -5; offset--)
    {
        int line_of_stone = (end_row - offset) * 2 + 3;
        int column_of_stone = (end_column + offset) * 4 + 16;

        move_cursor(line_of_stone, column_of_stone);

        // displays a brighter and different stone with a virtual terminal sequence to highlight the winning line
        if (gomoku_board[end_row][end_column] == 1)
            std::cout << "\x1B[97m@\x1B[0m";
        else
            std::cout << "\x1B[91m@\x1B[0m";
    }

    return;
}


/*
<Summary> :: replaces the contents in the message box with an ending message based on the battle result
<Parameter "winner"> :: the battle result (1 = player's victory, -1 = AI's victory, 0 = tie)
<Return> :: none
*/
void show_ending_message(int winner)
{
    move_cursor(34, 17);

    switch (winner)
    {
        case 1:
            std::cout << "        恭喜你贏了！     (再來一局)  (結束遊戲)        ";
            break;

        case -1:
            std::cout << "        可惜你輸了！     (再來一局)  (結束遊戲)        ";
            break;

        default:
            std::cout << "         雙方平手！      (再來一局)  (結束遊戲)        ";
            break;
    }

    return;
}


/*
<Summary> :: asks whether the player wants to play again and stores the information to a specified variable
<Parameter "ref_is_game_running"> :: a reference to the variable indicating whether the player wants to play again
<Return> :: the return value would be 0 if the function succeeds; otherwise the return value would be the line number where the error occurs
*/
int check_next_battle(bool &ref_is_game_running)
{
    HANDLE console_input_handle;
    bool is_button_pressed;

    // gets the console input handle, and exits the current function if the handle is not obtained successfully
    console_input_handle = GetStdHandle(STD_INPUT_HANDLE);
    if (console_input_handle == INVALID_HANDLE_VALUE)
        return __LINE__;

    // flushes the console input buffer, and exits the current function if the input records are not discarded successfully
    if (!FlushConsoleInputBuffer(console_input_handle))
        return __LINE__;

    is_button_pressed = false;
    while (!is_button_pressed)
    {
        INPUT_RECORD input_records[1];
        DWORD total_inputs_read;

        // reads one record from the console input buffer and removes it from the buffer, and exits the current function if the input record is not read successfully
        if (!ReadConsoleInput(console_input_handle, input_records, sizeof(input_records) / sizeof(INPUT_RECORD), &total_inputs_read))
            return __LINE__;

        if (input_records[0].EventType == MOUSE_EVENT)
        {
            MOUSE_EVENT_RECORD mouse_event {input_records[0].Event.MouseEvent};

            // highlights or unhiglights the "PLAY AGAIN" and "EXIT GAME" buttons when the player moves the mouse cursor
            if (mouse_event.dwEventFlags == MOUSE_MOVED)
            {
                if (mouse_event.dwMousePosition.Y == 33 && mouse_event.dwMousePosition.X >= 41 && mouse_event.dwMousePosition.X <= 50)
                {
                    move_cursor(34, 42);

                    // highlights the "PLAY AGAIN" button as bright yellow using a virtual terminal sequence
                    std::cout << "\x1B[93m(再來一局)\x1B[0m";
                }
                else if (mouse_event.dwMousePosition.Y == 33 && mouse_event.dwMousePosition.X >= 53 && mouse_event.dwMousePosition.X <= 62)
                {
                    move_cursor(34, 54);

                    // highlights the "EXIT GAME" button as bright yellow using a virtual terminal sequence
                    std::cout << "\x1B[93m(結束遊戲)\x1B[0m";
                }
                else
                {
                    move_cursor(34, 42);
                    std::cout << "(再來一局)  (結束遊戲)";
                }
            }
            // stores the selection to the specified variable when the player presses one of the buttons
            else if (mouse_event.dwEventFlags == 0 && mouse_event.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
            {
                if (mouse_event.dwMousePosition.Y == 33 && mouse_event.dwMousePosition.X >= 41 && mouse_event.dwMousePosition.X <= 50)
                {
                    ref_is_game_running = true;
                    is_button_pressed = true;
                }
                else if (mouse_event.dwMousePosition.Y == 33 && mouse_event.dwMousePosition.X >= 53 && mouse_event.dwMousePosition.X <= 62)
                {
                    ref_is_game_running = false;
                    is_button_pressed = true;
                }
            }
        }
    }

    return 0;
}