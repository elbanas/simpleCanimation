#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include "logo.h"

int main() {
    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;

    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) return 1;

    ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo);

    screensize = vinfo.yres_virtual * finfo.line_length;
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((intptr_t)fbp == -1) return 1;

    // 1. Paint the entire background white
    for (int y = 0; y < vinfo.yres; y++) {
        for (int x = 0; x < vinfo.xres; x++) {
            long int loc = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (y + vinfo.yoffset) * finfo.line_length;
            if (vinfo.bits_per_pixel == 32) {
                *((uint32_t*)(fbp + loc)) = 0xFFFFFFFF; // White
            } else if (vinfo.bits_per_pixel == 16) {
                *((uint16_t*)(fbp + loc)) = 0xFFFF;     // White
            }
        }
    }

    // 2. Calculate center coordinates
    int start_x = (vinfo.xres - LOGO_WIDTH) / 2;
    int start_y = (vinfo.yres - LOGO_HEIGHT) / 2;
    int i = 0;

    // 3. Draw the logo
    for (int y = 0; y < LOGO_HEIGHT; y++) {
        for (int x = 0; x < LOGO_WIDTH; x++) {
            int cx = start_x + x;
            int cy = start_y + y;

            // Prevent drawing outside screen boundaries
            if (cx >= 0 && cx < vinfo.xres && cy >= 0 && cy < vinfo.yres) {
                long int loc = (cx + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (cy + vinfo.yoffset) * finfo.line_length;
                uint32_t p = logo_pixels[i];

                if (vinfo.bits_per_pixel == 32) {
                    *((uint32_t*)(fbp + loc)) = p;
                } else if (vinfo.bits_per_pixel == 16) {
                    // Convert 32-bit ARGB to 16-bit RGB565
                    uint8_t r = (p >> 16) & 0xFF;
                    uint8_t g = (p >> 8) & 0xFF;
                    uint8_t b = p & 0xFF;
                    *((uint16_t*)(fbp + loc)) = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
                }
            }
            i++;
        }
    }

    // 4. Sleep forever to keep the image on screen
    while (1) {
        pause();
    }

    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}
