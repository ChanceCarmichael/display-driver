#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define USBC2HD4_IOC_MAGIC 'U'

struct usbc2hd4_display_info {
    uint8_t display_id;
    uint8_t connected;
    uint8_t enabled;
    uint8_t reserved;
    uint16_t width;
    uint16_t height;
    uint8_t refresh_rate;
    uint8_t orientation;
};

#define USBC2HD4_IOCTL_GET_DISPLAY_INFO _IOR(USBC2HD4_IOC_MAGIC, 3, struct usbc2hd4_display_info)
#define USBC2HD4_IOCTL_ENABLE_DISPLAY _IOW(USBC2HD4_IOC_MAGIC, 5, uint8_t)

int main(int argc, char **argv)
{
    const char *path = "/dev/usbc2hd40";
    int fd;
    int ret;
    uint8_t disp = 0;
    struct usbc2hd4_display_info info;

    if (argc > 1) path = argv[1];
    if (argc > 2) disp = (uint8_t)atoi(argv[2]);

    fd = open(path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open(%s) failed: %s\n", path, strerror(errno));
        return 1;
    }

    printf("Enabling display %u on %s\n", disp, path);
    ret = ioctl(fd, USBC2HD4_IOCTL_ENABLE_DISPLAY, &disp);
    if (ret == -1) {
        fprintf(stderr, "IOCTL ENABLE_DISPLAY failed: %s (%d)\n", strerror(errno), errno);
        close(fd);
        return 1;
    }
    printf("Enable ioctl returned %d\n", ret);

    memset(&info, 0, sizeof(info));
    info.display_id = disp;
    ret = ioctl(fd, USBC2HD4_IOCTL_GET_DISPLAY_INFO, &info);
    if (ret == -1) {
        fprintf(stderr, "IOCTL GET_DISPLAY_INFO failed: %s (%d)\n", strerror(errno), errno);
    } else {
        printf("Display %u info: connected=%u enabled=%u res=%ux%u@%u orientation=%u\n",
               info.display_id, info.connected, info.enabled, info.width, info.height, info.refresh_rate, info.orientation);
    }

    close(fd);
    return 0;
}
