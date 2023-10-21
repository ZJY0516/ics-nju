#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg)
{
    // nemu/src/device/vga.c
    // vgactl_port_base[0] = (screen_width() << 16) | screen_height();
    uint32_t screen_info = inl(VGACTL_ADDR);
    uint32_t Width = screen_info >> 16;
    uint32_t Height = screen_info & 0xFFFF;
    *cfg = (AM_GPU_CONFIG_T){
        .present = true,
        .has_accel = false,
        .width = Width,
        .height = Height,
        .vmemsz = Height * Width * sizeof(uint32_t) // vmemsz=?
    };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl)
{
    uint32_t screen_info = inl(VGACTL_ADDR);
    uint32_t Width = screen_info >> 16;
    uint32_t Height = screen_info & 0xFFFF;
    uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
    // if (!ctl->sync) {
    //     return;
    // }
    int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
    // if (w == 0 || h == 0)
    //     return;
    uint32_t *pixels = (uint32_t *)ctl->pixels;
    int len = (x + w >= Width) ? Width - x : w;
    for (int j = 0; j < h; j++) {
        if (y + j < Height) {
            for (int i = 0; i < len; i++) {
                fb[(j + y) * Width + x + i] = pixels[j * w + i];
            }
        }
    }
    if (ctl->sync) {
        outl(SYNC_ADDR, 1);
    }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) { status->ready = true; }
