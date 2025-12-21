#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

GtkWidget *display;
int has_operator = 0; // 연산자가 입력되었는지 확인하는 플래그

// 숫자 버튼 클릭 시
void on_number_clicked(GtkWidget *widget, gpointer data) {
    const char *num_str = (const char *)data;
    const char *current_text = gtk_entry_get_text(GTK_ENTRY(display));
    char new_text[100];

    // 처음에 "0"만 있으면 지우고 입력
    if (strcmp(current_text, "0") == 0) {
        gtk_entry_set_text(GTK_ENTRY(display), num_str);
    } else {
        sprintf(new_text, "%s%s", current_text, num_str);
        gtk_entry_set_text(GTK_ENTRY(display), new_text);
    }
}

// 연산자 버튼(+, -, *, /) 클릭 시
void on_operator_clicked(GtkWidget *widget, gpointer data) {
    if (has_operator) return; 

    const char *op_str = (const char *)data;
    const char *current_text = gtk_entry_get_text(GTK_ENTRY(display));
    char new_text[100];

    sprintf(new_text, "%s %s ", current_text, op_str); 
    gtk_entry_set_text(GTK_ENTRY(display), new_text);
    has_operator = 1;
}

// 초기화(C) 버튼 클릭 시
void on_clear_clicked(GtkWidget *widget, gpointer data) {
    gtk_entry_set_text(GTK_ENTRY(display), "0");
    has_operator = 0;
}

// 결과(=) 버튼 클릭 시
void on_equals_clicked(GtkWidget *widget, gpointer data) {
    const char *text = gtk_entry_get_text(GTK_ENTRY(display));
    double n1, n2, result = 0.0;
    char op;
    char result_str[100];

    // 문자열 파싱
    if (sscanf(text, "%lf %c %lf", &n1, &op, &n2) != 3) return;

    switch(op) {
        case '+': result = n1 + n2; break;
        case '-': result = n1 - n2; break;
        case '*': result = n1 * n2; break;
        case '/': if(n2 != 0) result = n1 / n2; break;
    }

    // 결과 출력
    if (result == (int)result) sprintf(result_str, "%d", (int)result);
    else sprintf(result_str, "%.2f", result);

    gtk_entry_set_text(GTK_ENTRY(display), result_str);
    has_operator = 0; // 계산 끝났으니 연산자 입력 가능 상태로
}

int main(int argc, char *argv[]) {
    GtkWidget *window, *grid;
    GtkWidget *button;
    
    // 버튼 배치표
    char *buttons[4][4] = {
        {"7", "8", "9", "/"},
        {"4", "5", "6", "*"},
        {"1", "2", "3", "-"},
        {"C", "0", "=", "+"}
    };

    gtk_init(&argc, &argv);

    // 윈도우 설정
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK Calculator");
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 300);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 그리드 레이아웃 생성
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // 결과 표시창 
    display = gtk_entry_new();
    gtk_entry_set_alignment(GTK_ENTRY(display), 1.0); // 오른쪽 정렬
    gtk_editable_set_editable(GTK_EDITABLE(display), FALSE); // 키보드 입력 방지
    gtk_entry_set_text(GTK_ENTRY(display), "0");
    gtk_grid_attach(GTK_GRID(grid), display, 0, 0, 4, 1); 

    // 버튼 생성 및 배치 
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            button = gtk_button_new_with_label(buttons[i][j]);
            gtk_widget_set_hexpand(button, TRUE);
            gtk_widget_set_vexpand(button, TRUE);

            if (strcmp(buttons[i][j], "C") == 0)
                g_signal_connect(button, "clicked", G_CALLBACK(on_clear_clicked), NULL);
            else if (strcmp(buttons[i][j], "=") == 0)
                g_signal_connect(button, "clicked", G_CALLBACK(on_equals_clicked), NULL);
            else if (buttons[i][j][0] >= '0' && buttons[i][j][0] <= '9')
                g_signal_connect(button, "clicked", G_CALLBACK(on_number_clicked), buttons[i][j]);
            else
                g_signal_connect(button, "clicked", G_CALLBACK(on_operator_clicked), buttons[i][j]);

            gtk_grid_attach(GTK_GRID(grid), button, j, i + 1, 1, 1);
        }
    }

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}