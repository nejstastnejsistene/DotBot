#ifndef MAIN_H
#define MAIN_H

#include "dots.h"

#include "libwebsockets.h"

#define DEFAULT_PORT 5000

/* Minimum amount of time that must pass between game updates. */
static const unsigned int min_update_interval = 500;

static volatile int force_exit = 0;
static struct libwebsocket_context *context;

struct per_session_data {
    long last_updated;
    int first;
    int score;
    grid_t grid;
};

static void init_session_data(struct per_session_data*);
static void tick(int*, char*, struct per_session_data*);

static int dotbot_stream_callback(struct libwebsocket_context*,
        struct libwebsocket*, enum libwebsocket_callback_reasons, void*, void*, size_t);

static struct libwebsocket_protocols protocols[] = {{
    "dotbot-stream",
    dotbot_stream_callback,
    sizeof(struct per_session_data),
}, {}};

#endif
