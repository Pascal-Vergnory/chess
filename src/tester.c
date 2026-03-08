#include <stdio.h>
#include <windows.h>
#include <unistd.h> // pour sleep

HANDLE ph[2];
FILE* fp[2];

void pipes_init( char* path0, char* path1)
{
    char cmd[100];
    char pipe_path[28];

    for (int side = 0; side < 2; side++) {

        // Create a pipe 'ph' for messages from the tester to the engine i 
        sprintf(pipe_path, "\\\\.\\pipe\\Pipe%d", side);
        ph[side] = CreateNamedPipe(pipe_path,
                            PIPE_ACCESS_DUPLEX,
                            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                            1,
                            1024 * 16,
                            1024 * 16,
                            NMPWAIT_USE_DEFAULT_WAIT,
                            NULL);
        if (ph[side] == NULL || ph[side] == INVALID_HANDLE_VALUE) {
            printf("failed to create named pipe %d\n", side);
            exit(1);
        }

        // Engine launch command includes argument to redirects Engine stdout to the pipe 'ph'
        sprintf(cmd, "%s >%s", side ? path1 : path0, pipe_path);

        // Start the Engine, with a pipe 'fp' from the tester to the engine stdin.
        fp[side] = popen(cmd, "w");

        // Wait for someone to connect to the pipe 'ph'
        if (!ConnectNamedPipe(ph[side], NULL)) {
            printf("failed to connect on named pipe\n");
            exit(1);
        }
    }
}

int get_from( int side, char* msg)
{
    HANDLE h = ph[side];
    char* ptr = msg;
    DWORD len;

    while (1) {
        if (ReadFile(
            h,
            ptr,                   // the data from the pipe will be put here
            1,                     // number of bytes allocated
            &len,                  // this will store number of bytes actually read
            NULL ) == 0) return 0; // not using overlapped IO
        if (*ptr == '\n') break;
        ptr ++;
        *ptr = 0;
    }
    ptr++;
    *ptr = 0;
    return 1;
}

void send_to( int side, char* msg)
{
    fprintf(fp[side], msg);
    fflush(fp[side]);
}

void display_board(char* board)
{
    char ch;
    printf("    a  b  c  d  e  f  g  h\n");
    printf("  +------------------------+\n");
    for (int l = 0; l < 8; l++) {
        printf("%d |", 8 - l);
        for (int c = 0; c < 8; c++) {
            ch = board[6 + 8*(7 - l) + c];
            if (ch == ' ') printf("   ");
            else printf(" %c ", ch);
        }
        printf("| %d\n", 8 - l);
    }
    printf("  +------------------------+\n");
    printf("    a  b  c  d  e  f  g  h\n");
}

int play_one_game( int side_to_play)
{
    int ply, side, result;
    char msg[128];

    for (side = 0; side < 2; side++) {
        send_to(side, "new\n");
        send_to(side, "random\n");
        send_to(side, "nopost\n");
        send_to(side, "stm 100\n");
    }

    // tell to 'side_to_start' to play 1st move
    strcpy(msg + 5, "go\n");

    for (ply = 1, side = side_to_play; ply < 300; ply++, side = 1 - side) {

        // send move to the other side
        send_to(side, msg + 5);

        // get move
        if (!get_from(side, msg)) break;
        if (msg[0] != 'm') break;
//        printf("%3d: %s", ply, msg);
//        putchar('.');
        printf(" %3d\b\b\b\b", ply);
    }
    //printf("\n%s\n", msg);

    // get resulting board and display the board
    //send_to(side, "getboard\n");
    //if (get_from(side, msg)) display_board(msg);

    // get evaluation from both sides
    send_to(1, "geteval\n");
    get_from(1, msg);
    //printf("Side 1 (%s) %s", side_to_play ? "whites" : "blacks", msg);
    result = atoi(msg + 5);

    send_to(0, "geteval\n");
    get_from(0, msg);
    //printf("Side 0 (%s) %s", side_to_play ? "blacks" : "whites", msg);
    result -= atoi(msg + 5);

    return result;
}


int main(int argc, char* argv[])
{
    (void) argc;

    int side, round, result;
    char msg[128];
    int wins[2] = { 0 };
    int advantages[2] = { 0 };

    pipes_init(argv[1], argv[2]);

    for (side = 0; side < 2; side++) if (!get_from(side, msg)) exit(1);  // get "ready" messages

    for (round = 0, side = 0; round <= 1000; round++, side = 1 - side) {
        result = play_one_game(side);
        if      (result > 300000) wins[1]++;
        else if (result > 100) advantages[1]++;
        else if (result < -300000) wins[0]++;
        else if (result < -100) advantages[0]++;

        printf("      Round %3d: Side 0: %3d wins, %3d advantages", round, wins[0], advantages[0]);
        printf(" Side 1: %3d wins, %3d advantages\r", wins[1], advantages[1]);
    }
    printf("\n");

    // Stop the chess engines
    for (side = 0; side < 2; side++) {
        send_to(side, "quit\n");
        CloseHandle(ph[side]);
    }
    return 0;
}