#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static uint32_t init_time = 0;
static int event_id = -1; // keyboard event
static struct dispinfo {
    int width;
    int height;
} dispinfo_t;

void get_dispinfo()
{
    int fd_dis = open("/proc/dispinfo", 0, 0);
    char tmp[30];
    read(fd_dis, (void *)tmp, 30);
    sscanf(tmp, "WIDTH:%d\nHEIGHT:%d\n", &dispinfo_t.width, &dispinfo_t.height);
    // dispinfo_t.height = 300;
    printf("width=%d, height=%d\n", dispinfo_t.width, dispinfo_t.height);
    assert(dispinfo_t.width && dispinfo_t.height);
}
static uint32_t get_milliseconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32_t)(tv.tv_sec * 1000 + (int)tv.tv_usec / 1000);
}

uint32_t NDL_GetTicks() { return get_milliseconds() - init_time; }

int NDL_PollEvent(char *buf, int len) { return read(event_id, buf, len); }

void NDL_OpenCanvas(int *w, int *h)
{
    if (*w == 0 && *h == 0) {
        *w = dispinfo_t.width;
        *h = dispinfo_t.height;
    }
    printf("w=%d, h=%d", *w, *h);
    if (getenv("NWM_APP")) {
        int fbctl = 4;
        fbdev = 5;
        screen_w = *w;
        screen_h = *h;
        char buf[64];
        int len = sprintf(buf, "%d %d", screen_w, screen_h);
        // let NWM resize the window and create the frame buffer
        write(fbctl, buf, len);
        while (1) {
            // 3 = evtdev
            int nread = read(3, buf, sizeof(buf) - 1);
            if (nread <= 0)
                continue;
            buf[nread] = '\0';
            if (strcmp(buf, "mmap ok") == 0)
                break;
        }
        close(fbctl);
    }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {}

void NDL_OpenAudio(int freq, int channels, int samples) {}

void NDL_CloseAudio() {}

int NDL_PlayAudio(void *buf, int len) { return 0; }

int NDL_QueryAudio() { return 0; }

int NDL_Init(uint32_t flags)
{
    if (getenv("NWM_APP")) {
        evtdev = 3;
    }
    init_time = get_milliseconds();
    event_id = open("/dev/events", 0, 0);
    // FILE *fp = fopen("/proc/dispinfo", "r");
    // fscanf(fp, "WIDTH:%d\nHEIGHT:%d\n", &dispinfo_t.width,
    // &dispinfo_t.height);
    get_dispinfo();
    return 0;
}

void NDL_Quit()
{
    init_time = 0;
    close(event_id);
}
