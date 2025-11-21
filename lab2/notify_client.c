#include <stdio.h>
#include <stdlib.h>
#include <systemd/sd-bus.h>

int main(int argc, char *argv[]) {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    const char *reply_msg;
    int ret;

    const char *msg_to_send = (argc > 1) ? argv[1] : "건물에 불이 났습니다.!";

    ret = sd_bus_default_user(&bus);
    if (ret < 0) {
        fprintf(stderr, "Failed to connect to user bus: %s\n", strerror(-ret));
        goto finish;
    }

    printf("[Client] 서버에 알림 전송 중: %s\n", msg_to_send);
    
    ret = sd_bus_call_method(bus,
                             "org.example.NotifyService",    /* 서비스 이름 */
                             "/org/example/NotifyService",   /* 객체 경로 */
                             "org.example.NotifyService",    /* 인터페이스 이름 */
                             "SendNotification",             /* 호출할 메소드 */
                             &error,                         /* 에러 저장소 */
                             &m,                             /* 응답 메시지 저장소 */
                             "s",                            /* 보낼 데이터 타입 (String) */
                             msg_to_send);                   /* 보낼 데이터 */

    if (ret < 0) {
        fprintf(stderr, "Failed to issue method call: %s\n", error.message);
        goto finish;
    }

    ret = sd_bus_message_read(m, "s", &reply_msg);
    if (ret < 0) {
        fprintf(stderr, "Failed to parse response message: %s\n", strerror(-ret));
        goto finish;
    }

    printf("[Client] 서버 응답: %s\n", reply_msg);

finish:
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
    sd_bus_unref(bus);

    return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}