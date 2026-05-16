#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define USBC2HD4_IOC_MAGIC 'U'

struct usbc2hd4_device_info {
    uint16_t vendor_id;
    uint16_t product_id;
    uint8_t num_displays;
    uint8_t reserved[3];
    char product_name[32];
    char serial_number[32];
};

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

struct usbc2hd4_resolution_cmd {
    uint8_t display_id;
    uint8_t reserved[3];
    uint16_t width;
    uint16_t height;
    uint8_t refresh_rate;
    uint8_t reserved2;
};

#define USBC2HD4_IOCTL_GET_DEVICE_INFO _IOR(USBC2HD4_IOC_MAGIC, 1, struct usbc2hd4_device_info)
#define USBC2HD4_IOCTL_ENUMERATE_DISPLAYS _IO(USBC2HD4_IOC_MAGIC, 2)
#define USBC2HD4_IOCTL_GET_DISPLAY_INFO _IOR(USBC2HD4_IOC_MAGIC, 3, struct usbc2hd4_display_info)

int main(int argc, char **argv)
{
    const char *path = "/dev/usbc2hd40";
    int fd;
    int ret;
    struct usbc2hd4_device_info dinfo;
    struct usbc2hd4_display_info pinfo;

    if (argc > 1) path = argv[1];

    fd = open(path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open(%s) failed: %s\n", path, strerror(errno));
        return 1;
    }

    memset(&dinfo, 0, sizeof(dinfo));
    ret = ioctl(fd, USBC2HD4_IOCTL_GET_DEVICE_INFO, &dinfo);
    if (ret == -1) {
        fprintf(stderr, "IOCTL GET_DEVICE_INFO failed: %s (%d)\n", strerror(errno), errno);
    } else {
        printf("IOCTL GET_DEVICE_INFO returned %d\n", ret);
        printf("  vendor=0x%04x product=0x%04x num_displays=%u\n",
               dinfo.vendor_id, dinfo.product_id, dinfo.num_displays);
        printf("  product='%s' serial='%s'\n", dinfo.product_name, dinfo.serial_number);
    }

    ret = ioctl(fd, USBC2HD4_IOCTL_ENUMERATE_DISPLAYS);
    if (ret == -1) {
        fprintf(stderr, "IOCTL ENUMERATE_DISPLAYS failed: %s (%d)\n", strerror(errno), errno);
    } else {
        printf("IOCTL ENUMERATE_DISPLAYS returned %d\n", ret);
    }

    memset(&pinfo, 0, sizeof(pinfo));
    pinfo.display_id = 0;
    ret = ioctl(fd, USBC2HD4_IOCTL_GET_DISPLAY_INFO, &pinfo);
    if (ret == -1) {
        fprintf(stderr, "IOCTL GET_DISPLAY_INFO failed: %s (%d)\n", strerror(errno), errno);
    } else {
        printf("IOCTL GET_DISPLAY_INFO returned %d\n", ret);
        printf("  display_id=%u connected=%u enabled=%u res=%ux%u@%u orientation=%u\n",
               pinfo.display_id, pinfo.connected, pinfo.enabled, pinfo.width, pinfo.height, pinfo.refresh_rate, pinfo.orientation);
    }

    close(fd);
    return 0;
}
