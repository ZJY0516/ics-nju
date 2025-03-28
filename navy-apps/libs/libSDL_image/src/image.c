#define SDL_malloc malloc
#define SDL_free free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface *IMG_Load_RW(SDL_RWops *src, int freesrc)
{
    assert(src->type == RW_TYPE_MEM);
    assert(freesrc == 0);
    return NULL;
}

SDL_Surface *IMG_Load(const char *filename)
{

    FILE *fp = fopen(filename, "r");
    printf("filename:%s\n", filename);
    assert(fp);

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buf = (char *)malloc(size);
    assert(buf);

    if (fread(buf, 1, size, fp) != size) {
        fclose(fp);
        free(buf);
        assert(0);
        return NULL; // 文件读取失败
    }

    fclose(fp);

    SDL_Surface *surface = STBIMG_LoadFromMemory((unsigned char *)buf, size);

    free(buf);

    return surface;
}

int IMG_isPNG(SDL_RWops *src) { return 0; }

SDL_Surface *IMG_LoadJPG_RW(SDL_RWops *src) { return IMG_Load_RW(src, 0); }

char *IMG_GetError() { return "Navy does not support IMG_GetError()"; }
