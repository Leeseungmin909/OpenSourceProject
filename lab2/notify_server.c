#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <systemd/sd-bus.h>

static int method_send_notification(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
    const char *message;
    char reply[100];
    int ret;

    ret = sd_bus_message_read(m, "s", &message);
    if (ret < 0) {
        fprintf(stderr, "Failed to parse parameters: %s\n", strerror(-ret));
        return ret;
    }

    printf("[Server] 알림 수신: %s\n", message);

    sprintf(reply, "알림 '%s'가 성공적으로 처리되었습니다.", message);

    return sd_bus_reply_method_return(m, "s", reply);
}

static const sd_bus_vtable notify_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD_WITH_ARGS("SendNotification",
                            SD_BUS_ARGS("s", message),
                            SD_BUS_RESULT("s", result),
                            method_send_notification,
                            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END
};

int main(int argc, char *argv[]) {
    sd_bus_slot *slot = NULL;
    sd_bus *bus = NULL;
    int ret;

    ret = sd_bus_default_user(&bus);
    if (ret < 0) {
        fprintf(stderr, "Failed to connect to user bus: %s\n", strerror(-ret));
        goto finish;
    }

    ret = sd_bus_add_object_vtable(bus,
                                   &slot,
                                   "/org/example/NotifyService",  /* 객체 경로 */
                                   "org.example.NotifyService",   /* 인터페이스 이름 */
                                   notify_vtable,
                                   NULL);
    if (ret < 0) {
        fprintf(stderr, "Failed to issue method call: %s\n", strerror(-ret));
        goto finish;
    }

    ret = sd_bus_request_name(bus, "org.example.NotifyService", 0);
    if (ret < 0) {
        fprintf(stderr, "Failed to acquire service name: %s\n", strerror(-ret));
        goto finish;
    }

    printf("[Server] 알림 서비스가 시작되었습니다...\n");

    for (;;) {
        ret = sd_bus_process(bus, NULL);
        if (ret < 0) {
            fprintf(stderr, "Failed to process bus: %s\n", strerror(-ret));
            goto finish;
        }
        if (ret > 0) 
            continue;

        ret = sd_bus_wait(bus, (uint64_t) -1);
        if (ret < 0) {
            fprintf(stderr, "Failed to wait on bus: %s\n", strerror(-ret));
            goto finish;
        }
    }

finish:
    sd_bus_slot_unref(slot);
    sd_bus_unref(bus);
    return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}