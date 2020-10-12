#include <gtk/gtk.h>
#include "gobang.h"
#include "csession.h"
#include "message.h"

/* 棋子信息结构 */
struct chess_info{
    short x,y;
    GtkWidget * image;
};

/* 棋子回调数据 */
struct chess_data{
    struct chess_info info;
    CLIENT_SESSION * session;
};

/* 全局变量方便回调函数修改 */
struct chess_info chess_graph[16][16];

/* 状态栏 */
GtkWidget * status_label;

/* 判断是否可以开始游戏 */
gboolean is_start_game=TRUE;

#define CHESS_INFO(object) ((struct chess_info *)object)

struct gb_board game_gb;

static GtkWidget * get_chess(const gchar * filename){
    GtkWidget * image = gtk_image_new_from_file(filename);
    return image;
}

/* 改变棋子在图形界面的图像 */
static gboolean change_chess(short x,short y,int type){\
    GtkImage * image;
    image=GTK_IMAGE(chess_graph[x-1][y-1].image);
    g_return_val_if_fail(image,FALSE);
    switch (type){
        case 1:
            g_object_set(image,"file","./resource/black_chess.png",NULL);
            break;
        case 0:
            g_object_set(image,"file","./resource/white_chess.png",NULL);
            break;
        default:
            g_object_set(image,"file","./resource/white.png",NULL);
            break;
    }
    return TRUE;
}
/*  初始化 */
static void chess_init(struct gb_board * cur_board){
    gb_init_board(cur_board,15,15);
    int i,j;
    for (i=1;i<=15;++i){
        for (j=1;j<=15;++j){
            change_chess(i,j,-1);
        }
    }
}

/* 显示获胜方 */
static gboolean winner_show(int flag){
    if (flag==-1) return FALSE;
    switch(flag){
        case -2:
            gtk_label_set_text(GTK_LABEL(status_label),"平局");
        break;
        case 1:
            gtk_label_set_text(GTK_LABEL(status_label),"白棋胜");
        break;
        case 0:
            gtk_label_set_text(GTK_LABEL(status_label),"黑棋胜");
        break;
    }
    is_start_game=FALSE;
    return TRUE;
}

/* 状态标签回调函数 */
static gboolean status_show(int flag){
    switch(flag) {
        case 0:
            gtk_label_set_text(GTK_LABEL(status_label),"当前轮到白方下棋");
        case 1:
            gtk_label_set_text(GTK_LABEL(status_label),"当前轮到黑方下棋");
    }
    return TRUE;
}

/* 菜单重新开始回调函数 */
static gboolean chess_restart(GtkWidget *widget,gpointer data){
    gtk_label_set_text(GTK_LABEL(status_label),"游戏开始");
    chess_init(&game_gb);
    is_start_game=TRUE;
    return TRUE;
}

/* 悔棋回调函数 */
static gboolean chess_back(GtkWidget *widget,gpointer data){
    if (!is_start_game || game_gb.gb_turn<1) return TRUE;
    change_chess(game_gb.gb_chess_manual[game_gb.gb_turn].x,game_gb.gb_chess_manual[game_gb.gb_turn].y,-1);
    gb_back_turn(&game_gb);
    change_chess(game_gb.gb_chess_manual[game_gb.gb_turn].x,game_gb.gb_chess_manual[game_gb.gb_turn].y,-1);
    gb_back_turn(&game_gb);
    return TRUE;
}

/* 响应鼠标点击 */
static gboolean chess_click(GtkWidget *widget,GdkEventButton *event,gpointer data){
    if (!is_start_game) return TRUE;
    struct gb_coord chess_pos;
    struct chess_data * info = (struct chess_data *)(data);
    chess_pos.x=info->info.x,chess_pos.y=info->info.y;
    // if (gb_place_judge(&game_gb,&chess_pos)){
    //     change_chess(chess_pos.x,chess_pos.y,game_gb.gb_cur_is);
    //     gb_nxt_turn(&game_gb,&chess_pos);
    //     if (winner_show(gb_win_judge(&game_gb))) return TRUE;
    //     status_show(game_gb.gb_cur_is);
    //     gb_auto_nxt_turn(&game_gb,&chess_pos,3);
    //     change_chess(chess_pos.x,chess_pos.y,game_gb.gb_cur_is);
    //     gb_nxt_turn(&game_gb,&chess_pos);
    //     if (winner_show(gb_win_judge(&game_gb))) return TRUE;
    //     status_show(game_gb.gb_cur_is);
    // }
    /* 发送下一步的信息 */
    struct Message_Type type = {2};
    char message_buffer[65535];
    bzero(message_buffer,sizeof(message_buffer));
    memcpy(message_buffer,&type,sizeof(type));
    memcpy(message_buffer+sizeof(type),&chess_pos,sizeof(chess_pos));
    info->session->send(info->session,message_buffer);
    return TRUE;
}

typedef struct GSENDDATA{
    CLIENT_SESSION * session;
    GtkEntry * entry;
};

static void send_message(GtkButton *button,
               gpointer   user_data){
    struct GSENDDATA * data = (struct GSENDDATA *)user_data;
    char * text = (char *)gtk_entry_get_text(data->entry);
    if (strlen(text)<=0){
        gtk_entry_set_text(data->entry,"");
        return;
    }
    char message_buffer[65535];
    bzero(message_buffer,sizeof(message_buffer));
    struct Message_Type type = {1}; 
    memcpy(message_buffer,&type,sizeof(type));
    struct Message message;
    bzero(&message,sizeof(struct Message));
    sprintf(message.buffer,"%s",text);
    memcpy(message_buffer+sizeof(type),&message,sizeof(message));
    data->session->send(data->session,message_buffer);
    gtk_entry_set_text(data->entry,"");
}


GtkTextBuffer * buffer;

pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;

/* Receive Event CallBack */
static void rev_event(CLIENT_SESSION * session,void * data){
    
    GtkTextIter * iter=malloc(sizeof(GtkTextIter));
    struct Message_Type type = {0};
    memcpy(&type,data,sizeof(type));   
    char message_buffer[1024];
    bzero(message_buffer,sizeof(message_buffer));
    struct Message message;
    struct gb_coord chess_pos;
    switch (type.type)
    {
    case 1:       
        memcpy(&message,data+sizeof(type),sizeof(message));
        pthread_mutex_lock(&buffer_lock);
        gtk_text_buffer_get_end_iter(buffer,iter);
        gtk_text_buffer_insert(buffer,iter,message.buffer,-1);
        gtk_text_buffer_get_end_iter(buffer,iter);
        gtk_text_buffer_insert(buffer,iter,"\n",-1);
        pthread_mutex_unlock(&buffer_lock);
        break;
    case 2:
        memcpy(&chess_pos,data+sizeof(type),sizeof(chess_pos));
         if (gb_place_judge(&game_gb,&chess_pos)){
        change_chess(chess_pos.x,chess_pos.y,game_gb.gb_cur_is);
        gb_nxt_turn(&game_gb,&chess_pos);
        if (winner_show(gb_win_judge(&game_gb))) break;
        status_show(game_gb.gb_cur_is);
        // gb_auto_nxt_turn(&game_gb,&chess_pos,3);
        // change_chess(chess_pos.x,chess_pos.y,game_gb.gb_cur_is);
        // gb_nxt_turn(&game_gb,&chess_pos);
        // if (winner_show(gb_win_judge(&game_gb))) return TRUE;
        // status_show(game_gb.gb_cur_is);
        break;
    }       
    default:
        break;
    }
    
}


/* 应用窗口 */
static void activate(GtkApplication *app, gpointer user_data) {
    // Init Session
    CLIENT_SESSION * session = client_session_init("localhost",5538);
    pthread_mutex_init(&buffer_lock,NULL);
    session->rev_event=&rev_event;
    // g_print("看到此条信息证明控制台未关闭");
    GtkWidget *window;
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW (window), "五子棋");
    gtk_widget_set_size_request(GTK_WIDGET(window),700,450);
    gtk_window_set_resizable(GTK_WINDOW(window),gtk_false());

    GtkWidget * chess_board;
    chess_board = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(chess_board),4);
    gtk_grid_set_row_spacing(GTK_GRID(chess_board),4);

    GtkWidget * fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window),fixed);
    /* 初始化五子棋界面 */
    GtkWidget * image_chess_board = gtk_image_new_from_file("./resource/board.png");
    gint i,j;
    for (i=0;i<15;++i){
        for (j=0;j<15;++j){
            GtkWidget * event_box=gtk_event_box_new();
            chess_graph[i][j].y=j+1;
            chess_graph[i][j].x=i+1;
            chess_graph[i][j].image=get_chess("./resource/white.png");
            gtk_container_add(GTK_CONTAINER(event_box),chess_graph[i][j].image);
            gtk_grid_attach(GTK_GRID(chess_board),event_box,i,j,1,1);
            struct chess_data * data = malloc(sizeof(struct chess_data));
            bzero(data,sizeof(data));
            data->info = chess_graph[i][j];
            data->session = session;
            g_signal_connect(G_OBJECT(event_box),"button_press_event",G_CALLBACK(chess_click),data);
        }
    }
    /* 初始化 */
    chess_init(&game_gb);
    status_label = gtk_label_new("游戏开始");
    buffer = gtk_text_buffer_new(NULL);
    GtkWidget * scrolled_window = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW (scrolled_window),250);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled_window),360);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), 
                                  GTK_POLICY_AUTOMATIC, 
                                  GTK_POLICY_AUTOMATIC); 
    

    /* Chat Text Box */
    gtk_text_buffer_set_text(buffer,"",-1);
    GtkWidget * text = gtk_text_view_new_with_buffer(buffer);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text), GTK_WRAP_WORD); 
    gtk_text_view_buffer_to_window_coords(GTK_TEXT_VIEW(text),GTK_TEXT_WINDOW_TEXT,180,400,NULL,NULL);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text),FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text),FALSE);
    gtk_container_add (GTK_CONTAINER (scrolled_window), 
                                         text);
    gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 1);

    GtkWidget * entry = gtk_entry_new();
    GtkWidget * button = gtk_button_new_with_label("发送信息");
    



    gtk_fixed_put(GTK_FIXED(fixed),image_chess_board,0,25);
    gtk_fixed_put(GTK_FIXED(fixed),chess_board,0,25);
    gtk_fixed_put(GTK_FIXED(fixed),status_label,0,430);
    gtk_fixed_put(GTK_FIXED(fixed),scrolled_window,430,25);
    gtk_fixed_put(GTK_FIXED(fixed),entry,430,395);
    gtk_fixed_put(GTK_FIXED(fixed),button,620,396);


    /* 创建一个水平的box */
    GtkWidget * menu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_fixed_put(GTK_FIXED(fixed),menu_box,0,0);

    GtkWidget * menu_bar,* menu_restart,* menu_back;
    menu_bar=gtk_menu_bar_new();
    gtk_widget_set_hexpand (menu_bar, TRUE);
    gtk_box_pack_start (GTK_BOX (menu_box),menu_bar, FALSE, TRUE, 0);

    menu_restart=gtk_menu_item_new_with_label("开始游戏");
    gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), menu_restart);

    menu_back = gtk_menu_item_new_with_label("悔棋");
    gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), menu_back);


    /* 信号连接菜单 */
    g_signal_connect(GTK_MENU_ITEM(menu_restart),"activate",G_CALLBACK(chess_restart),NULL);
    g_signal_connect(GTK_MENU_ITEM(menu_back),"activate",G_CALLBACK(chess_back),NULL);
    struct GSENDDATA * send_data = malloc(sizeof(struct GSENDDATA));
    bzero(send_data,sizeof(send_data));
    send_data->session=session;
    send_data->entry=GTK_ENTRY(entry);
    g_signal_connect(GTK_BUTTON(button),"clicked",G_CALLBACK(send_message),send_data);


    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("GoBang.app", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION (app), argc, argv);
    g_object_unref(app);

    return status;
}