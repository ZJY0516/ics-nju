#include <am.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR (AUDIO_ADDR + 0x14)

void __am_audio_init() {}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg)
{
    cfg->present = true;
    // outl(AUDIO_SBUF_SIZE_ADDR, cfg->bufsize);
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl)
{
    outl(AUDIO_FREQ_ADDR, ctrl->freq);
    outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
    outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
    outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat)
{
    stat->count = inl(AUDIO_COUNT_ADDR);
}
static uint32_t buf_pos = 0;
void __am_audio_play(AM_AUDIO_PLAY_T *ctl)
{
    uint32_t len = (ctl->buf.end - ctl->buf.start) / sizeof(uint8_t);
    // why uint8_t
    uint32_t size = inl(AUDIO_SBUF_SIZE_ADDR) / sizeof(uint8_t);
    uint32_t cnt = inl(AUDIO_COUNT_ADDR);

    while (len > size - cnt) {
        size = inl(AUDIO_SBUF_SIZE_ADDR) / sizeof(uint8_t);
        cnt = inl(AUDIO_COUNT_ADDR);
    }
    uint8_t *audio = ctl->buf.start;
    uint8_t *buffer = (uint8_t *)AUDIO_SBUF_ADDR;
    for (int i = 0; i < len; i++) {
        // write 0 lots of times
        buffer[buf_pos] = audio[i];
        buf_pos = (buf_pos + 1) % size;
    }
    // putch('1');
    outl(AUDIO_COUNT_ADDR, inl(AUDIO_COUNT_ADDR) + len);
    // memset(0)?
}
