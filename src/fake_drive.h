// fake_drive.h

typedef void (*fd_notify_cb_t)(uint8_t);
void fake_drive_init();
void set_fd_notify_cb(fd_notify_cb_t fn);
