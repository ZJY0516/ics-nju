#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
#define MULTIPROGRAM_YIELD() yield()
#else
#define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) [AM_KEY_##key] = #key,

static const char *keyname[256]
    __attribute__((used)) = {[AM_KEY_NONE] = "NONE", AM_KEYS(NAME)};

static AM_GPU_CONFIG_T gpu_config;
static AM_GPU_FBDRAW_T gpu_fbdraw __attribute__((unused));

size_t serial_write(const void *buf, size_t offset, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        putch(*((char *)buf + i));
    }
    return len;
}

size_t events_read(void *buf, size_t offset, size_t len)
{
    AM_INPUT_KEYBRD_T event;
    ioe_read(AM_INPUT_KEYBRD, &event);
    if (event.keycode == AM_KEY_NONE) {
        *(char *)buf = '\0';
        return 0;
    }
    if (event.keydown) {
        sprintf(buf, "kd %s\n", keyname[event.keycode]);
    } else {
        sprintf(buf, "ku %s\n", keyname[event.keycode]);
    }
    // snprintf(buf, 3, "1234");
    //  should use snprintf. TODO
    size_t re = strlen(buf);
    assert(re <= len);
    return re;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len)
{
    int width = gpu_config.width, height = gpu_config.height;
    sprintf(buf, "WIDTH:%d\nHEIGHT:%d\n", width, height);
    // len
    return strlen(buf);
}

size_t fb_write(const void *buf, size_t offset, size_t len) { return 0; }

void init_device()
{
    Log("Initializing devices...");
    ioe_init();
    ioe_read(AM_GPU_CONFIG, &gpu_config);
    printf("ioe: width=%d, height=%d\n", gpu_config.width, gpu_config.height);
}
