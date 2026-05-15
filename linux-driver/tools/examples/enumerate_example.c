/*
 * enumerate_example.c - Example program demonstrating display enumeration
 * 
 * Compile: gcc -o enumerate_example enumerate_example.c -I/usr/local/include \
 *         -L/usr/local/lib -lusbc2hd4
 * Run: ./enumerate_example
 */

#include <stdio.h>
#include <stdlib.h>
#include <usbc2hd4_lib.h>

int main(int argc, char *argv[])
{
    usbc2hd4_handle_t dev;
    int num_displays;
    int i;
    
    printf("USBC2HD4 Display Enumeration Example\n");
    printf("Library version: %s\n\n", usbc2hd4_lib_version());
    
    /* Open first available device */
    dev = usbc2hd4_open();
    if (!dev) {
        fprintf(stderr, "Failed to open device\n");
        return 1;
    }
    
    /* Get device information */
    usbc2hd4_device_info_t dev_info;
    if (usbc2hd4_get_device_info(dev, &dev_info) == 0) {
        printf("Device Information:\n");
        printf("  Product: %s\n", dev_info.product_name);
        printf("  Serial: %s\n", dev_info.serial_number);
        printf("  Vendor ID: 0x%04X\n", dev_info.vendor_id);
        printf("  Product ID: 0x%04X\n", dev_info.product_id);
        printf("\n");
    }
    
    /* Enumerate displays */
    num_displays = usbc2hd4_enumerate_displays(dev);
    if (num_displays < 0) {
        fprintf(stderr, "Failed to enumerate displays: %s\n",
                usbc2hd4_get_error_string(num_displays));
        usbc2hd4_close(dev);
        return 1;
    }
    
    printf("Connected Displays: %d\n\n", num_displays);
    
    /* Get info for each display */
    for (i = 0; i < num_displays; i++) {
        usbc2hd4_display_info_t display_info;
        int ret;
        
        ret = usbc2hd4_get_display_info(dev, i, &display_info);
        if (ret < 0) {
            printf("Display %d: Error getting info (%s)\n", i,
                   usbc2hd4_get_error_string(ret));
            continue;
        }
        
        printf("Display %d:\n", i);
        printf("  Connected: %s\n", display_info.connected ? "Yes" : "No");
        printf("  Enabled: %s\n", display_info.enabled ? "Yes" : "No");
        printf("  Resolution: %ux%u@%uHz\n",
               display_info.width, display_info.height,
               display_info.refresh_rate);
        printf("  Orientation: %s\n",
               display_info.orientation == 0 ? "Landscape" : "Portrait");
        printf("\n");
    }
    
    usbc2hd4_close(dev);
    return 0;
}
