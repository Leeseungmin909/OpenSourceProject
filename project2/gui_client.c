#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdint.h>
#include <locale.h> 

#define BUF_SIZE 1024
#define NAME_SIZE 20

int sock;
char name[NAME_SIZE];
char ip[20];
int port;

GtkWidget *login_window;
GtkWidget *chat_window;
GtkWidget *id_entry, *pw_entry;
GtkWidget *view, *entry;
GtkWidget *room_label;
GtkTextBuffer *buffer;

void create_chat_interface();
void *recv_msg(void *arg);
int connect_to_server();

struct MsgUpdate {
    char str[BUF_SIZE];
};

gboolean update_ui(gpointer data) {
    struct MsgUpdate *update = (struct MsgUpdate *)data;
    GtkTextIter end;
    if (buffer != NULL) {
        gtk_text_buffer_get_end_iter(buffer, &end);
        gtk_text_buffer_insert(buffer, &end, update->str, -1);
        gtk_text_buffer_insert(buffer, &end, "\n", -1);
        GtkTextMark *mark = gtk_text_buffer_create_mark(buffer, NULL, &end, FALSE);
        gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(view), mark);
    }
    free(update);
    return FALSE;
}

// ---------------------------------------------------
// 알림창 함수
// ---------------------------------------------------
void show_alert(GtkWidget *parent, const char *msg_text) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
                                  GTK_DIALOG_MODAL,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_OK,
                                  "%s", msg_text);
    gtk_dialog_run(GTK_DIALOG(dialog)); 
    gtk_widget_destroy(dialog);        
}

void show_info(GtkWidget *parent, const char *msg_text) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
                                  GTK_DIALOG_MODAL,
                                  GTK_MESSAGE_INFO,
                                  GTK_BUTTONS_OK,
                                  "%s", msg_text);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// ---------------------------------------------------
// 회원가입 (Register)
// ---------------------------------------------------
void on_signup_confirm(GtkWidget *widget, gpointer data) {
    GtkWidget **entries = (GtkWidget **)data;
    GtkWidget *dialog = gtk_widget_get_toplevel(entries[0]);
    const char *new_id = gtk_entry_get_text(GTK_ENTRY(entries[0]));
    const char *new_pw = gtk_entry_get_text(GTK_ENTRY(entries[1]));

    if (strlen(new_id) == 0 || strlen(new_pw) == 0) {
        show_alert(dialog, "아이디와 비밀번호를 모두 입력해주세요.");
        return;
    }

    int reg_sock = connect_to_server();
    if (reg_sock == -1) {
        show_alert(dialog, "서버 연결 실패!");
        return;
    }

    char req[50];
    sprintf(req, "REGISTER %s %s", new_id, new_pw);
    write(reg_sock, req, strlen(req));

    char res[100];
    int len = read(reg_sock, res, sizeof(res)-1);
    if (len > 0) res[len] = 0; else strcpy(res, "알 수 없는 오류");
    close(reg_sock);

    if (strncmp(res, "OK", 2) == 0) {
        show_info(dialog, "회원가입 성공! 로그인 해주세요.");
        gtk_widget_destroy(dialog); 
    } else {
        show_alert(dialog, "회원가입 실패 (ID 중복 등)"); 
    }
}

void on_signup_btn_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *content_area;
    GtkWidget *entry_id, *entry_pw;
    GtkWidget *btn_reg;
    
    dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "회원가입");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(login_window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 250, 200);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    entry_id = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_id), "새 아이디");
    gtk_container_add(GTK_CONTAINER(content_area), entry_id);
    
    entry_pw = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_pw), "새 비밀번호");
    gtk_entry_set_visibility(GTK_ENTRY(entry_pw), FALSE);
    gtk_container_add(GTK_CONTAINER(content_area), entry_pw);
    
    btn_reg = gtk_button_new_with_label("계정 생성");
    gtk_container_add(GTK_CONTAINER(content_area), btn_reg);

    GtkWidget **entries = malloc(sizeof(GtkWidget*) * 2);
    entries[0] = entry_id;
    entries[1] = entry_pw;

    g_signal_connect(btn_reg, "clicked", G_CALLBACK(on_signup_confirm), entries);

    gtk_widget_show_all(dialog);
}

// ---------------------------------------------------
// 로그인 (Login)
// ---------------------------------------------------
void on_login_clicked(GtkWidget *widget, gpointer data) {
    const char *id_text = gtk_entry_get_text(GTK_ENTRY(id_entry));
    const char *pw_text = gtk_entry_get_text(GTK_ENTRY(pw_entry));

    if (strlen(id_text) == 0 || strlen(pw_text) == 0) {
        show_alert(login_window, "아이디와 비밀번호를 입력해주세요.");
        return;
    }

    sock = connect_to_server();
    if (sock == -1) {
        show_alert(login_window, "서버 연결 실패! 서버가 켜져 있는지 확인하세요.");
        return; 
    }

    char login_info[50];
    sprintf(login_info, "LOGIN %s %s", id_text, pw_text);
    write(sock, login_info, strlen(login_info));

    char res[100];
    int len = read(sock, res, sizeof(res) - 1);
    
    if (len <= 0) {
        close(sock);
        show_alert(login_window, "서버 응답 없음.");
        return;
    }
    
    res[len] = 0;

    if (strncmp(res, "OK", 2) == 0) {
        strncpy(name, id_text, NAME_SIZE - 1);
        
        gtk_widget_hide(login_window); 
        
        create_chat_interface();          
        
        pthread_t rcv_thread;
        pthread_create(&rcv_thread, NULL, recv_msg, (void *)&sock);
    } else {
        close(sock); 
        show_alert(login_window, res); 
    }
}

// ---------------------------------------------------
// 네트워크 및 UI 로직
// ---------------------------------------------------
int connect_to_server() {
    int s = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);
    if (connect(s, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        return -1;
    return s;
}

void on_room_clicked(GtkWidget *widget, gpointer data) {
    int room_num = (intptr_t)data;
    char cmd[50];
    if (room_num == 0) sprintf(cmd, "/join 0");
    else sprintf(cmd, "/join %d", room_num);
    write(sock, cmd, strlen(cmd));
    
    char label_text[50];
    if (room_num == 0) sprintf(label_text, "현재: 로비");
    else sprintf(label_text, "현재: %d번 방", room_num);
    gtk_label_set_text(GTK_LABEL(room_label), label_text);
}

void *recv_msg(void *arg) {
    int sock = *((int *)arg);
    char msg[BUF_SIZE];
    int str_len;
    while (1) {
        str_len = read(sock, msg, BUF_SIZE - 1);
        if (str_len <= 0) return (void *)-1;
        msg[str_len] = 0;
        
        if (strncmp(msg, "[FILE]", 6) == 0) {
             char filename[100];
             sscanf(msg, "[FILE] %s", filename);
             char save_name[120];
             sprintf(save_name, "recv_%s", filename);
             FILE *fp = fopen(save_name, "wb");
             if (fp) {
                 struct MsgUpdate *u = malloc(sizeof(struct MsgUpdate));
                 sprintf(u->str, ">>> 파일 저장됨: %s", save_name);
                 g_idle_add(update_ui, u);
                 fclose(fp);
             }
        } else {
            struct MsgUpdate *u = malloc(sizeof(struct MsgUpdate));
            strcpy(u->str, msg);
            g_idle_add(update_ui, u);
        }
    }
    return NULL;
}

void on_send_clicked(GtkWidget *widget, gpointer data) {
    const char *msg = gtk_entry_get_text(GTK_ENTRY(entry));
    if(strlen(msg)==0) return;
    write(sock, msg, strlen(msg));
    gtk_entry_set_text(GTK_ENTRY(entry), "");
}

void on_file_clicked(GtkWidget *widget, gpointer data) {
     GtkWidget *dialog = gtk_file_chooser_dialog_new("파일 전송", GTK_WINDOW(chat_window),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "_취소", GTK_RESPONSE_CANCEL,
                                         "_전송", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        char *base_name = g_path_get_basename(filename);
        char header[BUF_SIZE];
        sprintf(header, "[FILE] %s", base_name);
        write(sock, header, strlen(header));
        
        struct MsgUpdate *u = malloc(sizeof(struct MsgUpdate));
        sprintf(u->str, ">>> 파일 전송 중: %s", base_name);
        g_idle_add(update_ui, u);
        
        g_free(filename); g_free(base_name);
    }
    gtk_widget_destroy(dialog);
}

void create_chat_interface() {
    chat_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    char title[50]; sprintf(title, "오픈소스 메신저 - %s", name);
    gtk_window_set_title(GTK_WINDOW(chat_window), title);
    gtk_window_set_default_size(GTK_WINDOW(chat_window), 600, 500);
    g_signal_connect(chat_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *hbox_main = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(chat_window), hbox_main);

    GtkWidget *vbox_side = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox_main), vbox_side, FALSE, FALSE, 0);
    
    GtkWidget *lbl = gtk_label_new("채널 목록");
    gtk_box_pack_start(GTK_BOX(vbox_side), lbl, FALSE, FALSE, 10);
    
    GtkWidget *btn_lobby = gtk_button_new_with_label("로비");
    g_signal_connect(btn_lobby, "clicked", G_CALLBACK(on_room_clicked), (gpointer)0);
    gtk_box_pack_start(GTK_BOX(vbox_side), btn_lobby, FALSE, FALSE, 0);

    for(int i=1; i<=10; i++) {
        char buf[20]; sprintf(buf, "%d번 방", i);
        GtkWidget *b = gtk_button_new_with_label(buf);
        g_signal_connect(b, "clicked", G_CALLBACK(on_room_clicked), (gpointer)(intptr_t)i);
        gtk_box_pack_start(GTK_BOX(vbox_side), b, FALSE, FALSE, 0);
    }

    GtkWidget *vbox_chat = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox_main), vbox_chat, TRUE, TRUE, 0);
    
    room_label = gtk_label_new("현재: 로비");
    gtk_box_pack_start(GTK_BOX(vbox_chat), room_label, FALSE, FALSE, 5);
    
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox_chat), scrolled, TRUE, TRUE, 0);
    
    view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    gtk_container_add(GTK_CONTAINER(scrolled), view);

    GtkWidget *hbox_input = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox_chat), hbox_input, FALSE, FALSE, 5);
    
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "메시지 입력...");
    g_signal_connect(entry, "activate", G_CALLBACK(on_send_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox_input), entry, TRUE, TRUE, 0);
    
    GtkWidget *btn_send = gtk_button_new_with_label("전송");
    g_signal_connect(btn_send, "clicked", G_CALLBACK(on_send_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox_input), btn_send, FALSE, FALSE, 0);
    
    GtkWidget *btn_file = gtk_button_new_with_label("파일");
    g_signal_connect(btn_file, "clicked", G_CALLBACK(on_file_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox_input), btn_file, FALSE, FALSE, 0);

    gtk_widget_show_all(chat_window);
}


int main(int argc, char *argv[]) {
    setlocale(LC_ALL, ""); 

    if(argc != 3) { printf("사용법: %s <IP> <PORT>\n", argv[0]); exit(1); }
    strcpy(ip, argv[1]);
    port = atoi(argv[2]);

    gtk_init(&argc, &argv);

    login_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(login_window), "로그인");
    gtk_window_set_default_size(GTK_WINDOW(login_window), 250, 250);
    gtk_window_set_position(GTK_WINDOW(login_window), GTK_WIN_POS_CENTER);
    g_signal_connect(login_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);
    gtk_container_add(GTK_CONTAINER(login_window), vbox);
    
    GtkWidget *lbl = gtk_label_new("아이디와 비밀번호를 입력하세요");
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    id_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(id_entry), "아이디");
    gtk_box_pack_start(GTK_BOX(vbox), id_entry, FALSE, FALSE, 0);
    
    pw_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(pw_entry), "비밀번호");
    gtk_entry_set_visibility(GTK_ENTRY(pw_entry), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), pw_entry, FALSE, FALSE, 0);
    
    GtkWidget *btn_login = gtk_button_new_with_label("로그인");
    g_signal_connect(btn_login, "clicked", G_CALLBACK(on_login_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), btn_login, FALSE, FALSE, 5);
    
    GtkWidget *btn_signup = gtk_button_new_with_label("회원가입");
    g_signal_connect(btn_signup, "clicked", G_CALLBACK(on_signup_btn_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), btn_signup, FALSE, FALSE, 5);

    gtk_widget_show_all(login_window);
    gtk_main();
    return 0;
}