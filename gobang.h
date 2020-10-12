#ifndef GoBang_H
#define GoBang_H

#ifndef MAX
#define  MAX(a,b) (a>b?a:b)
#endif

#ifndef MIN
#define  MIN(a,b) (a<b?a:b)
#endif

#ifndef ABS
#define  ABS(a) (a>=0?a:-a)
#endif

#define GB_MAX_HEIGHT 15 /*棋盘最大的高*/
#define GB_MAX_WIDTH 15 /*棋盘最大的宽*/

#include <time.h> /* 调用系统时间库 */
#include <stdlib.h> /* 调用标准库 */
#include <limits.h> /* 调用常用常量库 */
#include <string.h>

/* 棋子类型 */
enum gb_chess_type {
    gb_black,
    gb_white,
    gb_left_top,
    gb_right_top,
    gb_top,
    gb_left,
    gb_right,
    gb_middle,
    gb_left_bottom,
    gb_right_bottom,
    gb_bottom
};

/*模式类型*/
enum gb_mode_type {
    gb_normal_mode,
    gb_ban_mode
};

/*分数分值*/
enum gb_score_type {
    gb_win5=10000000,
    gb_alive4=1000000,
    gb_along4=100000,
    gb_die4=0,
    gb_alive3=100000,
    gb_along3=1000,
    gb_die3=0,
    gb_alive2=100,
    gb_along2=10,
    gb_die2=0,
    gb_alive1=1
};

/* 坐标结构 */
struct gb_coord{
    short x,y;
};

/* 常数组方向向量 */
const struct gb_coord gb_dir[8]={
    {1,0},{-1,0},{0,1},{0,-1},{1,1},{-1,-1},{1,-1},{-1,1}
};

/* 定义棋盘类 */
struct gb_board{
    char gb_title[80];
    short gb_graph[GB_MAX_WIDTH+1][GB_MAX_HEIGHT+1];/*棋盘图形，起始（1，1）*/
    short gb_width,gb_height; /*长和高*/
    short gb_turn; /*回合数*/
    short gb_cur_is;/* 当前下的一方 */
     /* short gb_mode;  模式 */ /*禁手模式特殊情况很多，对禁手模式暂时不是很了解，暂时先保留 */
    struct gb_coord gb_chess_manual[GB_MAX_HEIGHT*GB_MAX_WIDTH]; /*棋谱*/
};

/* void gb_set_mode(struct gb_board * cur_board,short mode){  设置游戏模式
    cur_board->gb_mode = mode;
}*/

/* 判断初始棋盘位置的类型 */
short gb_init_chess_judge(struct gb_board * cur_board,struct gb_coord * cur_pos){
    short cur_type;
    if (cur_pos->y==1) cur_type=gb_left_top; /* 顶部 */
    else if (cur_pos->y==cur_board->gb_height) cur_type=gb_left_bottom; /* 底部 */
    else cur_type=gb_left; /* 中间 */
    if (cur_pos->x!=1){
        if (cur_pos->x==cur_board->gb_width) cur_type++; /* 右 */
            else cur_type+=2; /* 中间 */
    }
    return cur_type;
}

/*初始化棋盘*/
int gb_init_board(struct gb_board * cur_board,short width,short height){
    memset(cur_board->gb_title,0,sizeof(cur_board));
    cur_board->gb_height = height,cur_board->gb_width=width; /* 初始化高和宽 */
   /* cur_board->gb_mode = gb_normal_mode; 初始化模式 */
    cur_board->gb_cur_is = gb_white; /* 初始化第0回合 */
    cur_board->gb_turn=0;
    struct gb_coord pos;
    for (pos.y=1;pos.y<=height;++pos.y){ /* 初始化棋盘图形 */
        for (pos.x=1;pos.x<=width;++pos.x){
            cur_board->gb_graph[pos.x][pos.y]=gb_init_chess_judge(cur_board,&pos);
        }
    }
    return 0;
}

/* 判断点是否在符合棋盘范围 */
int gb_in_board_judge(struct gb_board * cur_board,struct gb_coord * cur_pos){
    if (cur_pos->x<1 || cur_pos->y<1 || cur_pos->x > cur_board->gb_width || cur_pos->y > cur_board->gb_height)
        return 0;
    return 1;
}

/* 判断点是否在棋盘上放置 */
int gb_place_judge(struct gb_board * cur_board,struct gb_coord * cur_pos){
    if (gb_in_board_judge(cur_board,cur_pos) && cur_board->gb_graph[cur_pos->x][cur_pos->y]>1){
        return 1;
    }
    return 0;
}

/* 是否获胜判断，是返回gb_white或gb_black，平局返回-2，否则返回-1*/
int gb_win_judge(struct gb_board * cur_board){
    short dir;
    struct gb_coord pos;
    int chess_count_alive3=0,chess_count_alive4=0;
    for (dir=0;dir<8;dir+=2){
        short chess_count=1,is_left_chess=0,is_right_chess=0; /* 计算单个方向的棋子数量，端点是否有对手棋子 */

        pos.x=cur_board->gb_chess_manual[cur_board->gb_turn].x+gb_dir[dir].x,
        pos.y=cur_board->gb_chess_manual[cur_board->gb_turn].y+gb_dir[dir].y;
        while (gb_in_board_judge(cur_board,&pos) && cur_board->gb_graph[pos.x][pos.y]==cur_board->gb_cur_is){
            chess_count++;
            pos.x+=gb_dir[dir].x,pos.y+=gb_dir[dir].y;
        };
        if (gb_in_board_judge(cur_board,&pos) && cur_board->gb_graph[pos.x][pos.y]!=(cur_board->gb_cur_is^1)) is_left_chess=1;  /* 左方向 */

        pos.x=cur_board->gb_chess_manual[cur_board->gb_turn].x+gb_dir[dir+1].x,
        pos.y=cur_board->gb_chess_manual[cur_board->gb_turn].y+gb_dir[dir+1].y;
        while (gb_in_board_judge(cur_board,&pos) && cur_board->gb_graph[pos.x][pos.y]==cur_board->gb_cur_is){
            chess_count++;
            pos.x+=gb_dir[dir+1].x,pos.y+=gb_dir[dir+1].y;
        };
        if (gb_in_board_judge(cur_board,&pos) && cur_board->gb_graph[pos.x][pos.y]!=(cur_board->gb_cur_is^1)) is_right_chess=1;  /* 右方向 */

       /* if (cur_board->gb_mode==gb_ban_mode && cur_board->gb_cur_is==gb_black){  禁手模式下计算活三、活四数量和长连情况
            if (chess_count==3 && is_left_chess+is_right_chess==2) ++chess_count_alive3;
            if (chess_count==4 && is_left_chess+is_right_chess==2) ++chess_count_alive4;
            if (chess_count>5) {return gb_white;}
        }*/
        if (chess_count>=5) return cur_board->gb_cur_is;
    }
    /*if (cur_board->gb_mode==gb_ban_mode && cur_board->gb_cur_is==gb_black)
    if (chess_count_alive3>1 || chess_count_alive4>1) return gb_white;  出现双活三及双活四以上的情况 */
    if (cur_board->gb_turn==cur_board->gb_height*cur_board->gb_width) return -2; /* 平局情况 */
    return -1;

}

/*获取这个点在棋盘上的评估分值 */
int gb_get_pos_score(struct gb_board * cur_board,struct gb_coord * cur_pos,short cur_type){
    short dir;
    int score_sum=0,chess_count_alive3=0,chess_count_alive4=0;/* 分数总值，连活三数量，连活四数量 */
    struct gb_coord pos;
    for (dir=0;dir<8;dir+=2){
        short chess_count=1,is_left_chess=0,is_right_chess=0; /* 计算单个方向的棋子数量，端点是否有对手棋子 */
        pos.x=cur_pos->x+gb_dir[dir].x,pos.y=cur_pos->y+gb_dir[dir].y;
        while (gb_in_board_judge(cur_board,&pos) && cur_board->gb_graph[pos.x][pos.y]==cur_type){
            chess_count++;
            pos.x+=gb_dir[dir].x,pos.y+=gb_dir[dir].y;
        };
        if (gb_in_board_judge(cur_board,&pos) && cur_board->gb_graph[pos.x][pos.y]!=(cur_type^1)) is_left_chess=1;  /* 左方向 */

        pos.x=cur_pos->x+gb_dir[dir+1].x,pos.y=cur_pos->y+gb_dir[dir+1].y;
        while (gb_in_board_judge(cur_board,&pos) && cur_board->gb_graph[pos.x][pos.y]==cur_type){
            chess_count++;
            pos.x+=gb_dir[dir+1].x,pos.y+=gb_dir[dir+1].y;
        };
        if (gb_in_board_judge(cur_board,&pos) && cur_board->gb_graph[pos.x][pos.y]!=(cur_type^1)) is_right_chess=1;  /* 右方向 */

        /*if (cur_board->gb_mode==gb_ban_mode && cur_type==gb_black){ 禁手模式下计算活三、活四数量和长连情况
            if (chess_count>5) {score_sum=0;break;}
            if (chess_count==3 && is_left_chess+is_right_chess==2) ++chess_count_alive3;
            if (chess_count==4 && is_left_chess+is_right_chess==2) ++chess_count_alive4;
        }*/
        if (chess_count>=5){ /* 获胜的分数直接退出 */
            score_sum=gb_win5;break;
        }
        switch (chess_count){ /* 根据棋数和端点数目判断连活三、连活4等等情况 */
        case 4:
            switch (is_left_chess+is_right_chess){
            case 2:score_sum+=gb_alive4;break;
            case 1:score_sum+=gb_along4;break;
            case 0:score_sum+=gb_die4;break;
            }break;
        case 3:
            switch (is_left_chess+is_right_chess){
            case 2:score_sum+=gb_alive3;break;
            case 1:score_sum+=gb_along3;break;
            case 0:score_sum+=gb_die3;break;
            }break;
        case 2:
            switch (is_left_chess+is_right_chess){
            case 2:score_sum+=gb_alive2;break;
            case 1:score_sum+=gb_along2;break;
            case 0:score_sum+=gb_die2;break;
            }break;
        case 1:
            score_sum+=gb_alive1;break;
        }
    }
   /* if (cur_board->gb_mode==gb_ban_mode && cur_type==gb_black)
    if (chess_count_alive3>1 || chess_count_alive4>1) return 0;*/
    return score_sum;
}

/* 更新棋盘下一步 */
int gb_nxt_turn(struct gb_board * cur_board,struct gb_coord * cur_pos){
    if (cur_board->gb_turn+1>cur_board->gb_width*cur_board->gb_height) return 0;
    cur_board->gb_turn++;
    cur_board->gb_cur_is^=1;
    cur_board->gb_chess_manual[cur_board->gb_turn]=*cur_pos;
    cur_board->gb_graph[cur_pos->x][cur_pos->y]=cur_board->gb_cur_is;
    return 1;
}

/* 回退棋谱,返回上一步 */
int gb_back_turn(struct gb_board * cur_board){
    if (cur_board->gb_turn==0) return 0;
    struct gb_coord pos=cur_board->gb_chess_manual[cur_board->gb_turn];
    cur_board->gb_turn--;
    cur_board->gb_cur_is^=1;
    cur_board->gb_graph[pos.x][pos.y]=gb_init_chess_judge(cur_board,&pos);
    return 1;
}

/* 深度优先搜索搜索最大利益值点 */
int gb_auto_nxt_turn(struct gb_board * cur_board,struct gb_coord * choose_pos,int depth){
    srand(time(NULL));
    struct gb_coord pos={8,8};
    if (cur_board->gb_turn==0){
        *choose_pos=pos;
        return 0;
    }
    int max_score=0,pos_count=0;
    for (pos.y=1;pos.y<=cur_board->gb_height;++pos.y){
        for (pos.x=1;pos.x<=cur_board->gb_width;++pos.x){
            if (cur_board->gb_graph[pos.x][pos.y]>1){
                int score=0; /* 该点的分值 */
                ++pos_count;
                score+=gb_get_pos_score(cur_board,&pos,cur_board->gb_cur_is^1);/* 假设自己下这个点获得的分值 */
                score+=gb_get_pos_score(cur_board,&pos,cur_board->gb_cur_is)/2;/* 对方下这个点的分值 */
                gb_nxt_turn(cur_board,&pos);
                if (score>max_score && depth>1 && cur_board->gb_turn<cur_board->gb_height*cur_board->gb_width){ /* 向下层搜索 */
                    struct gb_coord nxt_pos;
                    score-=gb_auto_nxt_turn(cur_board,&nxt_pos,depth-1); /* 扣去对手可以获得的利益值 */
                    if (score<0) score=0;
                }
                if (score>max_score || (score==max_score && rand()%pos_count==1)){ /* 更新点 */
                    *choose_pos=pos;
                    max_score=score;
                }
                gb_back_turn(cur_board); /* 复原 */
            }
        }
    }
    return max_score;
}
#endif