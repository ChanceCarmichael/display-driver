/*
 * set_resolution_example.c - Example program demonstrating resolution setting
 * 
 * Compile: gcc -o set_resolution_example set_resolution_example.c \
 *         -I/usr/local/include -L/usr/local/lib -lusbc2hd4
 * Run: ./set_resolution_example <display_id> <width> <height> <refresh_rate>
 */

#include <stdio.h>
#include <stdlib.h>
#include <usbc2hd4_lib.h>

int main(int argc, char *argv[])
{
    usbc2hd4_handle_t dev;
    int display_id;
    int width, height, refresh_rate;
    int ret;
    
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <display_id> <width> <height> <refresh_rate>\n",
                argv[0]);
        fprintf(stderr, "Example: %s 0 1920 1080 60\n", argv[0]);
        return 1;
    }
    
    display_id = atoi(argv[1]);
    width = atoi(argv[2]);
    height = atoi(argv[3]);
    refresh_rate = atoi(argv[4]);
    
    printf("Setting Display Resolution\n");
    printf("Display: %d\n", display_id);
    printf("Resolution: %dx%d @ %dHz\n", width, height, refresh_rate);
    
    /* Open device */
    dev = usbc2hd4_open();
    if (!dev) {
        fprintf(stderr, "Failed to open device\n");
        return 1;
    }
    
    /* Enable the display first */
    ret = usbc2hd4_enable_display(dev, display_id);
    if (ret < 0) {
        fprintf(stderr, "Failed to enable display: %s\n",
                usbc2hd4_get_error_string(ret));
        usbc2hd4_close(dev);
        return 1;
    }
    printf("Display enabled\n");
    
    /* Set resolution */
    ret = usbc2hd4_set_resolution(dev, display_id, width, height, refresh_rate);
    if (ret < 0) {
        fprintf(stderr, "Failed to set resolution: %s\n",
                usbc2hd4_get_error_string(ret));
        usbc2hd4_close(dev);
        return 1;
    }
    
    printf("Resolution set successfully\n");
    
    usbc2hd4_close(dev);
    return 0;
}
