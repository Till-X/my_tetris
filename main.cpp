#include <curses.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

int x,y,r,px,py,pr,shapes,colors;
int lines;
int score;
int my_key;
long start;
bool game=1;
int board[20][10];

int shape_type[7][4][10] = {
    //O
    {
        {0,0,1,1,0,1,0,1,  2,2},
        {0,0,1,1,0,1,0,1,  2,2},
        {0,0,1,1,0,1,0,1,  2,2},
        {0,0,1,1,0,1,0,1,  2,2}
    },
    //L
    {
        {0,1,2,2,0,0,0,1,  2,3},
        {0,1,1,1,2,0,1,2,  3,2},
        {0,0,1,2,0,1,1,1,  2,3},
        {0,0,0,1,0,1,2,0,  3,2}
    },
    //T
    {
        {0,1,1,1,1,0,1,2,  3,2},
        {0,1,1,2,0,0,1,0,  2,3},
        {0,0,0,1,0,1,2,1,  3,2},
        {0,1,1,2,1,0,1,1,  2,3}
    },
    //|
    {
        {0,1,2,3,0,0,0,0,  1,4},
        {0,0,0,0,0,1,2,3,  4,1},
        {0,1,2,3,0,0,0,0,  1,4},
        {0,0,0,0,0,1,2,3,  4,1}
    },
    //Z
    {
        {0,0,1,1,0,1,1,2,  3,2},
        {0,1,1,2,1,0,1,0,  2,3},
        {0,0,1,1,0,1,1,2,  3,2},
        {0,1,1,2,1,0,1,0,  2,3}

    },
    //反L
    {
        {0,1,2,2,1,1,0,1,  2,3},
        {0,1,1,1,0,0,1,2,  3,2},
        {0,0,1,2,0,1,0,0,  2,3},
        {0,0,0,1,0,1,2,2,  3,2}
    },
    //反Z
    {
        {0,0,1,1,1,2,0,1,  3,2},
        {0,1,1,2,0,0,1,1,  2,3},
        {0,0,1,1,1,2,0,1,  3,2},
        {0,1,1,2,0,0,1,1,  2,3}
    }

};

// create a new piece, don't remove old one (it has landed and should stick)
void new_piece() {
  y = py = 0;
  shapes = rand() % 7;
  r = pr = rand() % 4;
  x = px = rand() % (11 - shape_type[shapes][r][8]);
  colors = rand() % 7 + 1;
}

void set_piece(int x, int y, int r, int shapes, int color) {
  for (int i = 0; i < 4; i++) {
    board[shape_type[shapes][r][i] + y][shape_type[shapes][r][i+4] + x] = color;
  }
}

// move a piece from old (p*) coords to new
void update_piece() {
   set_piece(px, py, pr, shapes, 0);
   set_piece(px = x, py = y, pr = r, shapes, colors);
}

void frame(){
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
           move(1 + i, j * 2 + 1); // otherwise the box won't draw
           if(board[i][j] != 0){
                attron(A_REVERSE);
                attron(COLOR_PAIR(board[i][j]));
                printw("  ");
                attroff(COLOR_PAIR(board[i][j]));
                attroff(A_REVERSE);
            }
            else{
                attron(COLOR_PAIR(board[i][j]));
                printw("  ");
                attroff(COLOR_PAIR(board[i][j]));
            }
        }
    }
    move(21, 1);
    printw("Score: %d", score);
    refresh();
}

void remove_line(){
    for (int row = y; row <= y + shape_type[shapes][r][9]; row++) {
        lines = 1;
        for(int i = 0; i < 10; i++){
            lines *= board[row][i];
        }
        if(lines){
            for (int i = row - 1; i > 0; i--) {
                memcpy(&board[i + 1][0], &board[i][0], 40);
            }
            memset(&board[0][0], 0, 10);
            score++;
        }
    }
}

// check if placing p at (x,y,r) will be a collision
int check_hit(int x, int y, int r, int shapes) {
    int a=0;
    if (y + shape_type[shapes][r][9] > 20) {
        return 1;
    }
    set_piece(px, py, pr, shapes, 0);
    for (int i = 0; i < 4; i++) {
        a += board[shape_type[shapes][r][i] + y][shape_type[shapes][r][i+4] + x];
    } 
    if(a != 0){
        set_piece(px, py, pr, shapes, colors);
        return 1;
    }
    set_piece(px, py, pr, shapes, colors);
    return 0;
}

void runloop(){
    if(my_key == 'a' && x > 0 && !check_hit(x - 1,y,r,shapes)){
        x--;
    }
    if(my_key == 'd' && x+shape_type[shapes][r][8] <= 9 && !check_hit(x + 1,y,r,shapes)){
        x++;
    }
    if(my_key == 's') {
      while (!check_hit(x, y + 1,r,shapes)) {
        y++;
        update_piece();
      }
      remove_line();
      new_piece();
    }
    if(my_key == 'w'){
        ++r %= 4;
        while (x + shape_type[shapes][r][8] > 9) {
            x--;
        }
        if (check_hit(x,y,r,shapes)) {
            x = px;
            r = pr;
        }
    }
    if (my_key == 'q') {
      game = 0;
    }
    my_key = 0;
    update_piece();
    frame();
}

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if(ch != EOF){
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

void key_check(){
    char ch;
    if(kbhit()){
        my_key = getchar();
    }
    switch(ch){
        case 'w':my_key='w';break;
        case 's':my_key='s';break;
        case 'a':my_key='a';break;
        case 'd':my_key='d';break;
        case 'q':my_key='q';break;
    }
}

int main(){
    srand(time(0));
    initscr();
    start_color();
    // colours indexed by their position in the block
    for (int i = 1; i < 8; i++) {
        init_pair(i, i, 0);
    }
    new_piece();
    resize_term(22, 22);
    noecho();
    curs_set(0);
    box(stdscr, 0, 0);
    start = clock();
    while(game){
        key_check();
        runloop();
        if(clock()-start>=400000){
            if(check_hit(x, y + 1, r,shapes)){
                if(!y){
                    game = 0;
                }
                remove_line();
                new_piece();
            }else{
                y++;
                update_piece();
            }
            start = clock();
        }
    }
    endwin();
}
