#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {"NONE", _KEYS(keyname)};
#define NR_KEYS (sizeof(keyname) / sizeof(keyname[0]))
static uint8_t keystate[NR_KEYS] = {0};

int SDL_PushEvent(SDL_Event *ev) { return 0; }

int SDL_PollEvent(SDL_Event *ev)
{
    char buf[32];
    char key[16];
    if (NDL_PollEvent(buf, sizeof(buf)) == 0) {
        ev->type = SDL_KEYUP;
        return 0;
    }
    if (strncmp(buf, "kd", 2) == 0) {
        sscanf(buf + 3, "%s\n", key);
        ev->type = SDL_KEYDOWN;
    } else if (strncmp(buf, "ku", 2) == 0) {
        sscanf(buf + 3, "%s\n", key);
        ev->type = SDL_KEYUP;
    } else
        assert(0);
    for (int i = 0; i < NR_KEYS; i++) {
        if (strcmp(key, keyname[i]) == 0) {
            ev->key.keysym.sym = i;
            keystate[i] = (ev->type == SDL_KEYDOWN) ? 1 : 0;
            break;
        }
    }
    return 1;
}

int SDL_WaitEvent(SDL_Event *event)
{
    // nanos-lite/src/device events_read()
    // printf("wait event\n");
    char buf[32];
    char key[16];
    while (NDL_PollEvent(buf, 32) == 0) {
        event->type = SDL_KEYUP;
    };
    if (strncmp(buf, "kd", 2) == 0) {
        sscanf(buf + 3, "%s\n", key);
        event->type = SDL_KEYDOWN;
    } else if (strncmp(buf, "ku", 2) == 0) {
        sscanf(buf + 3, "%s\n", key);
        event->type = SDL_KEYUP;
    } else
        assert(0);
    for (int i = 0; i < NR_KEYS; i++) {
        if (strcmp(key, keyname[i]) == 0) {
            event->key.keysym.sym = i;
            break;
        }
    }
    return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask)
{
    return 0;
}

uint8_t *SDL_GetKeyState(int *numkeys) { return keystate; }
