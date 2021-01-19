#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/omapfb.h>

void drawColor(int width, int height, int color)
{
    uint16_t *canvas = new uint16_t[width * height];
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
			if (((x > 200) && (x <= 300) ) && ((y > 20) && (y <= 40) ) ) {
	            canvas[x + y * width] = 0x00FF;  
			}
			else {
	            canvas[x + y * width] = color;  
			}
        }
    }

    int fd;
    fd = open("/dev/fb1", O_WRONLY);
    write(fd, canvas, width * height * 2);
    close(fd);
    delete[] canvas;
}

void drawColor2(int width, int height, int color)
{
    int fd;
    fd = open("/dev/fb1", O_RDWR);
    uint16_t *frameBuffer = (uint16_t *)mmap(NULL, width * height * 2, PROT_WRITE, MAP_SHARED, fd, 0);
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            frameBuffer[x + y * width] = color; 
        }
    }

    msync(frameBuffer, width * height * 2, 0);
    munmap(frameBuffer, width * height * 2);
    close(fd);
}



int main()
{
    int width = 640;
    int height = 48;

    /* Fill solid color */
    drawColor(width, height, 0xFFFF);
}
