/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>

enum {
    reg_freq,
    reg_channels,
    reg_samples,
    reg_sbuf_size,
    reg_init,
    reg_count,
    nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;
// static uint32_t old_pos = 0;
static uint32_t sbuf_pos = 0;
void sdl_audio_callback(void *userdata, uint8_t *stream, int len)
{
    uint32_t cnt = audio_base[reg_count];
    memset(stream, 0, len);
    len = (len > cnt) ? cnt : len;
    uint32_t buf_size = audio_base[reg_sbuf_size];
    // putchar('2');
    if ((sbuf_pos + len) > buf_size) {
        memcpy(stream, sbuf + sbuf_pos, buf_size - sbuf_pos);
        memcpy(stream + (buf_size - sbuf_pos), sbuf + (buf_size - sbuf_pos),
               len - (buf_size - sbuf_pos));
    } else {
        memcpy(stream, sbuf + sbuf_pos, len);
    }
    // old_pos = sbuf_pos;
    sbuf_pos = (sbuf_pos + len) % buf_size;
    audio_base[reg_count] -= len;
}

static void audio_io_handler(uint32_t offset, int len, bool is_write)
{
    if (!audio_base[reg_init])
        return;
    // SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec s = {};
    s.format =
        AUDIO_S16SYS; // 假设系统中音频数据的格式总是使用16位有符号数来表示
    s.userdata = NULL; // 不使用
    s.freq = audio_base[reg_freq];
    s.channels = audio_base[reg_channels];
    s.samples = audio_base[reg_samples];
    s.callback = sdl_audio_callback;
    int ret = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (ret == 0) {
        SDL_OpenAudio(&s, NULL);
        SDL_PauseAudio(0);
    }
    audio_base[reg_init] = 0;
}

void init_audio()
{
    uint32_t space_size = sizeof(uint32_t) * nr_reg;
    audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
    add_pio_map("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size,
                audio_io_handler);
#else
    add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size,
                 audio_io_handler);
#endif

    sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
    add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
    audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
}
