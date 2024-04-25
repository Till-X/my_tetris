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

// 数据形式为: 『x1,x2,x3,x4,y1,y2,y3,y4,  W,H}
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

// 随机生成一个新的方块
void new_piece() {
  y = py = 0;
  shapes = rand() % 7;
  r = pr = rand() % 4;
  x = px = rand() % (11 - shape_type[shapes][r][8]);
  colors = rand() % 7 + 1;
}

// 在虚拟内存空间中绘制一个方块
void set_piece(int x, int y, int r, int shapes, int color) {
  for (int i = 0; i < 4; i++) {
    board[shape_type[shapes][r][i] + y][shape_type[shapes][r][i+4] + x] = color;
  }
}

// 清除当前 (px,py) 处的方块，并且在 (x,y) 处绘制新的方块
void update_piece() {
   set_piece(px, py, pr, shapes, 0);
   set_piece(px = x, py = y, pr = r, shapes, colors);
}

// 将虚拟内存中的数据更新到屏幕上
void frame(){
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
           move(1 + i, j * 2 + 1); 
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

// 检测是否排满一行，是否清除
void remove_line(){
    for (int row = y; row <= y + shape_type[shapes][r][9]; row++) {
    // 从当前方块所在的行y开始，循环至当前方块底部的行为止  注意：是以当前方块为一个基准进行判断的
        lines = 1;
        for(int i = 0; i < 10; i++){
            lines *= board[row][i]; // 利用乘法的操作进行判断一行中的每一个格子是否为空
        }
        if(lines){
            for (int i = row - 1; i > 0; i--) {
                memcpy(&board[i + 1][0], &board[i][0], 40);
                // 将上一行的内容复制到当前行的位置，将上面的行向下移动一格。这里使用了 memcpy 函数来进行内存拷贝。
            }
            memset(&board[0][0], 0, 10); // 最顶层进行清空，他的上一层是没有东西可以进行复制了的
            score++;
        }
    }
}

// 检测是否会发生碰撞，只是检测，并不会更新虚拟内存空间
int check_hit(int x, int y, int r, int shapes) {
    int a=0;
    if (y + shape_type[shapes][r][9] > 20) {
        return 1;
    }// 这里检查的是：当前y值是否已经到达底部 到达底部  返回1
    set_piece(px, py, pr, shapes, 0);// 先清空当前的方块，防止干扰检测
    for (int i = 0; i < 4; i++) {
        a += board[shape_type[shapes][r][i] + y][shape_type[shapes][r][i+4] + x];
    } 
    if(a != 0){
        set_piece(px, py, pr, shapes, colors);// 检测到碰撞后，需要恢复被清除的当前方块
        return 1;
    }
    set_piece(px, py, pr, shapes, colors);
    return 0;
}

// 处理按键的返回值，左移、右翼、变形、快速下降、退出游戏
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
        // 变形后撞墙，左移动避开
        while (x + shape_type[shapes][r][8] > 9) {
            x--;
        }
        // 无法避开的情况，恢复原来形状
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

// 由于Linux环境下没有kbhit函数，手动实现(该函数由chatgpt编写)
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

// 按键检测函数
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
    // 初始化颜色对  这里是七个形状，因此，也是对应着这里的7个颜色
    for (int i = 1; i < 8; i++) {
        init_pair(i, i, 0);// 使用前景色和背景色初始化一个颜色对。
    }
    new_piece();// 为俄罗斯方块游戏创建一个新的游戏块。
    resize_term(22, 22);// 调整终端屏幕的尺寸为 22x22
    noecho();// 禁止将输入的字符自动显示在屏幕上。
    curs_set(0);// 将光标可见性设置为不可见。用于控制终端光标可见性的函数调用 可以达到隐藏光标的能力
    box(stdscr, 0, 0);// 在标准终端窗口中周围绘制一个带有边框的方框。
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
