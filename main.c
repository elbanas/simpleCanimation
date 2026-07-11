#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mmap.h>
#include <sys/ioctl.h>
#include <stdint.h>

int main() {
    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;

    // Open the primary framebuffer
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) return 1;

    // Get fixed and variable screen information
    ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo);

    // Calculate memory size (width * height * bytes per pixel)
    screensize = vinfo.yres_virtual * finfo.line_length;

    // Map framebuffer to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((intptr_t)fbp == -1) return 1;

    int x = 0, y = 0;
    int dx = 5, dy = 5;
    int box_size = 50;

    // Animation Loop
    while (1) {
        // Clear screen (paint it black)
        for (int i = 0; i < screensize; i++) fbp[i] = 0x00;

        // Draw the bouncing box
        for (int cy = y; cy < y + box_size; cy++) {
            for (int cx = x; cx < x + box_size; cx++) {
                long int location = (cx + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) +
                                    (cy + vinfo.yoffset) * finfo.line_length;

                // Write a solid color (e.g., cyan)
                if (vinfo.bits_per_pixel == 32) {
                    *((uint32_t*)(fbp + location)) = 0x00FFFF; // ARGB
                } else if (vinfo.bits_per_pixel == 16) {
                    *((uint16_t*)(fbp + location)) = 0x07FF;   // RGB565
                }
            }
        }

        // Update position
        x += dx;
        y += dy;

        // Bounce off edges
        if (x < 0 || x > vinfo.xres - box_size) dx = -dx;
        if (y < 0 || y > vinfo.yres - box_size) dy = -dy;

        // Sleep for ~16ms (~60 FPS)
        usleep(16000);
    }

    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}
