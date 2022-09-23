#define SPXE_APPLICATION
#include <spxe.h>
#include <imgtool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH spxe.scrres.width
#define HEIGHT spxe.scrres.height
#define inside(x, y) ((x) >= 0 && (y) >= 0 && (x) < WIDTH && (y) < HEIGHT)

typedef struct ivec2 {
    int x, y;
} ivec2;

typedef struct Rect {
    ivec2 pos, size;
} Rect;

static inline int rectAt(const Rect rect, const ivec2 p)
{
    ivec2 r = {rect.size.x / 2, rect.size.y / 2}, v = rect.pos;
    return p.x >= v.x - r.x && p.x < v.x + r.x && p.y >= v.y - r.y && p.y < v.y + r.y;
}

static inline Px* bmpAt(const bmp_t* bmp, const ivec2 pos, const ivec2 p)
{
    if (p.x >= pos.x && p.x < pos.x + (int)bmp->width && 
        p.y >= pos.y && p.y < pos.y + (int)bmp->height) {
        return (Px*)px_at(bmp, p.x - pos.x, p.y - pos.y);
    }
    return NULL;
}

static void bmpDraw(Px* pixbuf, const bmp_t* bmp, const ivec2 pos)
{
    const int yend = pos.y + bmp->height;
    for (int y = pos.y; y < yend; ++y) {
        memcpy(
            pixbuf + y * WIDTH + pos.x, 
            px_at(bmp, 0, y - pos.y),
            bmp->width * sizeof(Px)
        );
    }
}

static bmp_t bmpGrad(void)
{
#define GRADSIZE 32
    const int len = 256 / GRADSIZE;
    bmp_t bmp = bmp_new(GRADSIZE, GRADSIZE, 4);
    for (int y = 0; y < GRADSIZE; ++y) {
        for (int x = 0; x < GRADSIZE; ++x) {
            *(Px*)px_at(&bmp, x, y) = (Px){x * len, y * len, 0, 255};
        }
    }
    return bmp;
}

int main(const int argc, const char** argv)
{
    int bmpwidth = 32, bmpheight = 32, outcount = 0;
    int scrwidth = 200, scrheight = 150;
    char fileout[0xff] = "image.png";

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-w")) {
            if (i + 1 == argc) {
                printf("Missing argument for %s option.\n", argv[i]);
                return EXIT_FAILURE;
            }
            bmpwidth = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-h")) {
            if (i + 1 == argc) {
                printf("Missing argument for %s option.\n", argv[i]);
                return EXIT_FAILURE;
            }
            bmpheight = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-x")) {
            if (i + 1 == argc) {
                printf("Missing argument for %s option.\n", argv[i]);
                return EXIT_FAILURE;
            }
            scrwidth = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-y")) {
            if (i + 1 == argc) {
                printf("Missing argument for %s option.\n", argv[i]);
                return EXIT_FAILURE;
            }
            scrheight = atoi(argv[++i]);
        }
        else strcpy(fileout, argv[i]);
    }

    bmp_t grad = bmpGrad();
    bmp_t bmp = bmp_new(bmpwidth, bmpheight, 4);
    memset(bmp.pixels, 255, bmp.width * bmp.height * bmp.channels);

    Px color = {0, 0, 0, 255};
    Px* pixbuf = spxeStart("spaint", 800, 600, scrwidth, scrheight);
    memset(pixbuf, 155, WIDTH * HEIGHT * sizeof(Px));

    const ivec2 center = {(WIDTH - bmp.width) / 2, (HEIGHT - bmp.height) / 2};
    const ivec2 i2zero = {0, 0};
    ivec2 mouse;

    printf(
        "spaint:\nimage size: %u x %u\nfile name: %s\npress P to save image\n",
        bmp.width, 
        bmp.height, 
        fileout
    );

    while (spxeRun(pixbuf)) {
        if (spxeKeyPressed(ESCAPE)) {
            break;
        }

        if (spxeKeyPressed(P)) {
            char buf[0xff], *s = fileout, *dot, num[0xf];
            sprintf(num, "%d", outcount++);
            while ((s = strchr(s, '.'))) { dot = s++; }
            size_t n = dot - fileout, m = strlen(dot), k = strlen(num);
            memcpy(buf, fileout, n);
            memcpy(buf + n, num, k);
            memcpy(buf + n + k, dot, m);
            buf[n + m + k] = 0;
            bmp_write(buf, &bmp);
            printf("saved file '%s'\n", buf);
        }

        spxeMousePos(&mouse.x, &mouse.y);

        memset(pixbuf, 155, WIDTH * HEIGHT * sizeof(Px));
        bmpDraw(pixbuf, &bmp, center);
        bmpDraw(pixbuf, &grad, i2zero);

        if (inside(mouse.x, mouse.y)) {
            pixbuf[mouse.y * WIDTH + mouse.x] = color;
            int mouseDown = spxeMouseDown(LEFT);
            
            Px* p = bmpAt(&bmp, center, mouse);
            if (mouseDown && p) {
                *p = color;
            }
            
            p = bmpAt(&grad, i2zero, mouse);
            if (mouseDown && p) {
                color = *p;
            }

        }
    }

    return spxeEnd(pixbuf);
}
