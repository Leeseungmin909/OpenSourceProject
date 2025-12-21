#include <gtk/gtk.h>
#include <stdio.h>

int count = 0;
GtkWidget *label; 

// 버튼 누르면 숫자 증가
void on_button_clicked(GtkWidget *widget, gpointer data) {
    count++;
    char buffer[50];
    
    sprintf(buffer, "Button clicked: %d times", count);
    gtk_label_set_text(GTK_LABEL(label), buffer);
}

void quit(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *vbox; 

    // GTK 초기화 
    gtk_init(&argc, &argv);

    // 윈도우 설정
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "My Simple GTK");
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 150);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    
    // X 버튼 누르면 꺼지게 연결
    g_signal_connect(window, "destroy", G_CALLBACK(quit), NULL);

    // 레이아웃 박스 생성 
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // 레이블 생성
    label = gtk_label_new("Button clicked: 0 times");
    gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);

    // 버튼 생성 및 이벤트 연결
    button = gtk_button_new_with_label("Click Me!");
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 10);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}