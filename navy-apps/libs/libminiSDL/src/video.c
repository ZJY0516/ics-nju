#include <NDL.h>
#include <sdl-video.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst,
                     SDL_Rect *dstrect)
{
    assert(dst && src);
    assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
    int is_malloc_srcrect = 0, is_malloc_dstrect = 0;
    if (srcrect == NULL) {
        srcrect = malloc(sizeof(SDL_Rect));
        srcrect->x = 0;
        srcrect->y = 0;
        srcrect->w = src->w;
        srcrect->h = src->h;
        is_malloc_srcrect = 1;
    }
    if (dstrect == NULL) {
        dstrect = malloc(sizeof(SDL_Rect));
        dstrect->x = 0;
        dstrect->y = 0;
        dstrect->w = dst->w;
        dstrect->h = dst->h;
        is_malloc_dstrect = 1;
    }
    uint32_t s_start = srcrect->y * src->w + srcrect->x;
    uint32_t d_start = dstrect->y * dst->w + dstrect->x;
    // // assert(srcrect->w == dstrect->w && srcrect->h == dstrect->h);
    uint32_t src_offset, dest_offset;
    if (dst->format->BitsPerPixel == 32) {
        for (int i = 0; i < srcrect->h; i++) {
            src_offset = s_start + i * src->w;
            dest_offset = d_start + i * dst->w;
            memcpy((uint32_t *)dst->pixels + dest_offset,
                   (uint32_t *)src->pixels + src_offset,
                   srcrect->w * sizeof(uint32_t));
            // pixels is uint8_t pointer!!!
        }
    } else if (dst->format->BitsPerPixel == 8) {
        for (int i = 0; i < srcrect->h; i++) {
            src_offset = s_start + i * src->w;
            dest_offset = d_start + i * dst->w;
            memcpy(dst->pixels + dest_offset, src->pixels + src_offset,
                   srcrect->w * sizeof(uint8_t));
        }
    } else
        assert(0);
    if (is_malloc_dstrect) {
        free(dstrect);
    }
    if (is_malloc_srcrect) {
        free(srcrect);
    }
}

void SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color)
{
    // assert(dst->format->BitsPerPixel == 32);
    SDL_Rect tmp;
    assert(dst);
    if (dstrect == NULL) {
        dstrect = &tmp;
        dstrect->x = 0;
        dstrect->y = 0;
        dstrect->w = dst->w;
        dstrect->h = dst->h;
    }
    int x = (int)dstrect->x, y = (int)dstrect->y, w = (int)dstrect->w,
        h = (int)dstrect->h;
    if (dst->format->BitsPerPixel == 32) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++)
                *((uint32_t *)dst->pixels + ((y + i) * dst->w + j)) = color;
        }
    } else if (dst->format->BitsPerPixel == 8) {
        assert(dst->format->palette);
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++)
                *((uint8_t *)dst->pixels + ((y + i) * dst->w + j)) = color;
            // dst->format->palette
            //     ->colors[*((uint8_t *)dst->pixels + ((y + i) * dst->w + j))]
            //     .val = color;
        }
    } else
        assert(0);
    // SDL_UpdateRect(dst, x, y, w, h);
}

void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h)
{
    if (w == 0 && h == 0) {
        w = s->w;
        h = s->h;
    }
    assert(w && h);
    uint32_t len = w * h;
    uint32_t *buf = malloc(sizeof(uint32_t) * len);
    uint32_t start = y * s->w + x;
    uint32_t i = 0;

    uint32_t temp, pixel;
    uint8_t red, green, blue, alpha;
    SDL_PixelFormat *fmt = s->format;
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            if (s->format->BitsPerPixel == 32) {
                pixel = *((uint32_t *)s->pixels + start + row * s->w + col);

                /* Get Red component */
                temp = pixel & fmt->Rmask;  /* Isolate red component */
                temp = temp >> fmt->Rshift; /* Shift it down to 8-bit */
                temp = temp << fmt->Rloss;  /* Expand to a full 8-bit number */
                red = (uint8_t)temp;

                /* Get Green component */
                temp = pixel & fmt->Gmask;  /* Isolate green component */
                temp = temp >> fmt->Gshift; /* Shift it down to 8-bit */
                temp = temp << fmt->Gloss;  /* Expand to a full 8-bit number */
                green = (uint8_t)temp;

                /* Get Blue component */
                temp = pixel & fmt->Bmask;  /* Isolate blue component */
                temp = temp >> fmt->Bshift; /* Shift it down to 8-bit */
                temp = temp << fmt->Bloss;  /* Expand to a full 8-bit number */
                blue = (uint8_t)temp;

                /* Get Alpha component */
                temp = pixel & fmt->Amask;  /* Isolate alpha component */
                temp = temp >> fmt->Ashift; /* Shift it down to 8-bit */
                temp = temp << fmt->Aloss;  /* Expand to a full 8-bit number */
                alpha = (uint8_t)temp;

                buf[i++] = (alpha << 24) | (red << 16) | (green << 8) | (blue);
            } else if (s->format->BitsPerPixel == 8) {
                assert(s->format->palette);
                SDL_Color color =
                    s->format->palette
                        ->colors[*(s->pixels + start + row * s->w + col)];
                buf[i++] =
                    color.a << 24 | color.r << 16 | color.g << 8 | color.b;
            } else
                assert(0);
        }
    }
    assert(i == len);
    NDL_DrawRect(buf, x, y, w, h);
    // remeber free
    free(buf);
}

// APIs below are already implemented.

static inline int maskToShift(uint32_t mask)
{
    switch (mask) {
    case 0x000000ff:
        return 0;
    case 0x0000ff00:
        return 8;
    case 0x00ff0000:
        return 16;
    case 0xff000000:
        return 24;
    case 0x00000000:
        return 24; // hack
    default:
        assert(0);
    }
}

SDL_Surface *SDL_CreateRGBSurface(uint32_t flags, int width, int height,
                                  int depth, uint32_t Rmask, uint32_t Gmask,
                                  uint32_t Bmask, uint32_t Amask)
{
    assert(depth == 8 || depth == 32);
    SDL_Surface *s = malloc(sizeof(SDL_Surface));
    assert(s);
    s->flags = flags;
    s->format = malloc(sizeof(SDL_PixelFormat));
    assert(s->format);
    if (depth == 8) {
        s->format->palette = malloc(sizeof(SDL_Palette));
        assert(s->format->palette);
        s->format->palette->colors = malloc(sizeof(SDL_Color) * 256);
        assert(s->format->palette->colors);
        memset(s->format->palette->colors, 0, sizeof(SDL_Color) * 256);
        s->format->palette->ncolors = 256;
    } else {
        s->format->palette = NULL;
        s->format->Rmask = Rmask;
        s->format->Rshift = maskToShift(Rmask);
        s->format->Rloss = 0;
        s->format->Gmask = Gmask;
        s->format->Gshift = maskToShift(Gmask);
        s->format->Gloss = 0;
        s->format->Bmask = Bmask;
        s->format->Bshift = maskToShift(Bmask);
        s->format->Bloss = 0;
        s->format->Amask = Amask;
        s->format->Ashift = maskToShift(Amask);
        s->format->Aloss = 0;
    }

    s->format->BitsPerPixel = depth;
    s->format->BytesPerPixel = depth / 8;

    s->w = width;
    s->h = height;
    s->pitch = width * depth / 8;
    assert(s->pitch == width * s->format->BytesPerPixel);

    if (!(flags & SDL_PREALLOC)) {
        s->pixels = malloc(s->pitch * height);
        assert(s->pixels);
    }

    return s;
}

SDL_Surface *SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height,
                                      int depth, int pitch, uint32_t Rmask,
                                      uint32_t Gmask, uint32_t Bmask,
                                      uint32_t Amask)
{
    SDL_Surface *s = SDL_CreateRGBSurface(SDL_PREALLOC, width, height, depth,
                                          Rmask, Gmask, Bmask, Amask);
    assert(pitch == s->pitch);
    s->pixels = pixels;
    return s;
}

void SDL_FreeSurface(SDL_Surface *s)
{
    if (s != NULL) {
        if (s->format != NULL) {
            if (s->format->palette != NULL) {
                if (s->format->palette->colors != NULL)
                    free(s->format->palette->colors);
                free(s->format->palette);
            }
            free(s->format);
        }
        if (s->pixels != NULL && !(s->flags & SDL_PREALLOC))
            free(s->pixels);
        free(s);
    }
}

SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags)
{
    if (flags & SDL_HWSURFACE)
        NDL_OpenCanvas(&width, &height);
    return SDL_CreateRGBSurface(flags, width, height, bpp, DEFAULT_RMASK,
                                DEFAULT_GMASK, DEFAULT_BMASK, DEFAULT_AMASK);
}

void SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst,
                     SDL_Rect *dstrect)
{
    assert(src && dst);
    assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
    assert(dst->format->BitsPerPixel == 8);

    int x = (srcrect == NULL ? 0 : srcrect->x);
    int y = (srcrect == NULL ? 0 : srcrect->y);
    int w = (srcrect == NULL ? src->w : srcrect->w);
    int h = (srcrect == NULL ? src->h : srcrect->h);

    assert(dstrect);
    if (w == dstrect->w && h == dstrect->h) {
        /* The source rectangle and the destination rectangle
         * are of the same size. If that is the case, there
         * is no need to stretch, just copy. */
        SDL_Rect rect;
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
        SDL_BlitSurface(src, &rect, dst, dstrect);
    } else {
        assert(0);
    }
}

void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors,
                    int firstcolor, int ncolors)
{
    assert(s);
    assert(s->format);
    assert(s->format->palette);
    assert(firstcolor == 0);

    s->format->palette->ncolors = ncolors;
    memcpy(s->format->palette->colors, colors, sizeof(SDL_Color) * ncolors);

    if (s->flags & SDL_HWSURFACE) {
        assert(ncolors == 256);
        for (int i = 0; i < ncolors; i++) {
            uint8_t r = colors[i].r;
            uint8_t g = colors[i].g;
            uint8_t b = colors[i].b;
        }
        SDL_UpdateRect(s, 0, 0, 0, 0);
    }
}

static void ConvertPixelsARGB_ABGR(void *dst, void *src, int len)
{
    int i;
    uint8_t(*pdst)[4] = dst;
    uint8_t(*psrc)[4] = src;
    union {
        uint8_t val8[4];
        uint32_t val32;
    } tmp;
    int first = len & ~0xf;
    for (i = 0; i < first; i += 16) {
#define macro(i)                                                               \
    tmp.val32 = *((uint32_t *)psrc[i]);                                        \
    *((uint32_t *)pdst[i]) = tmp.val32;                                        \
    pdst[i][0] = tmp.val8[2];                                                  \
    pdst[i][2] = tmp.val8[0];

        macro(i + 0);
        macro(i + 1);
        macro(i + 2);
        macro(i + 3);
        macro(i + 4);
        macro(i + 5);
        macro(i + 6);
        macro(i + 7);
        macro(i + 8);
        macro(i + 9);
        macro(i + 10);
        macro(i + 11);
        macro(i + 12);
        macro(i + 13);
        macro(i + 14);
        macro(i + 15);
    }

    for (; i < len; i++) {
        macro(i);
    }
}

SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt,
                                uint32_t flags)
{
    assert(src->format->BitsPerPixel == 32);
    assert(src->w * src->format->BytesPerPixel == src->pitch);
    assert(src->format->BitsPerPixel == fmt->BitsPerPixel);

    SDL_Surface *ret =
        SDL_CreateRGBSurface(flags, src->w, src->h, fmt->BitsPerPixel,
                             fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

    assert(fmt->Gmask == src->format->Gmask);
    assert(fmt->Amask == 0 || src->format->Amask == 0 ||
           (fmt->Amask == src->format->Amask));
    ConvertPixelsARGB_ABGR(ret->pixels, src->pixels, src->w * src->h);

    return ret;
}

uint32_t SDL_MapRGBA(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b,
                     uint8_t a)
{
    assert(fmt->BytesPerPixel == 4);
    uint32_t p = (r << fmt->Rshift) | (g << fmt->Gshift) | (b << fmt->Bshift);
    if (fmt->Amask)
        p |= (a << fmt->Ashift);
    return p;
}

int SDL_LockSurface(SDL_Surface *s) { return 0; }

void SDL_UnlockSurface(SDL_Surface *s) {}
