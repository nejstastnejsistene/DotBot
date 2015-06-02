#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include <libwebsockets.h>
#include <json/json.h>

#include "dots.h"

#define DEFAULT_PORT 5000

/* Minimum amount of time that must pass between game updates. */
#define MIN_UPDATE_INTERVAL 500

static volatile int force_exit = 0;
static struct libwebsocket_context *context;

struct per_session_data {
    long last_updated;
    int new_game;
    int score;
    grid_t grid;
};

static void init_session_data(struct per_session_data *data) {
    data->new_game = 1;
    data->score = 0;
    memset(data->grid, 0, sizeof(data->grid));
    fill_grid(data->grid, EMPTY);
}

static json_object *json_grid(grid_t grid) {
    json_object *jgrid = json_object_new_array();
    char row_buf[NUM_COLS+1];
    int row, col;
    for (row = 0; row < NUM_ROWS; row++) {
        for (col = 0; col < NUM_COLS; col++) {
            row_buf[col] = " RYGBV??"[GET_COLUMN_COLOR(grid[col], row)];
        }
        row_buf[col] = 0;
        json_object_array_add(jgrid, json_object_new_string(row_buf));
    }
    return jgrid;
}

static json_object *json_path(int path_length, path_t path) {
    json_object *jpath = json_object_new_array();
    int i;
    for (i = 0; i < path_length; i++) {
        json_object *jcoord = json_object_new_array();
        json_object_array_add(jcoord, json_object_new_int(INDEX_ROW(path[i])));
        json_object_array_add(jcoord, json_object_new_int(INDEX_COL(path[i])));
        json_object_array_add(jpath, jcoord);
    }
    return jpath;
}

static void tick(int *len, char *buf, struct per_session_data *data) {
    int path_length = 0;
    path_t path;

    json_object *obj;
    const char *s;

    /* If it's a new game, only send the grid and don't compute a move. */
    if (data->new_game) {
        data->new_game = 0;
    } else {
        struct timeval tv;
        long ms;
        mask_t move;

        gettimeofday(&tv, NULL);
        ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
        if ((ms - data->last_updated) < MIN_UPDATE_INTERVAL) {
            return;
        }
        data->last_updated = ms;

        move = naive_choose_move(data->grid);
        apply_move(data->grid, move);
        fill_grid(data->grid, CYCLE_COLOR(move));
        mask_to_path(move, &path_length, path);
    }

    obj = json_object_new_object();
    json_object_object_add(obj, "grid", json_grid(data->grid));
    if (path_length) {
        json_object_object_add(obj, "path", json_path(path_length, path));
    }

    s = json_object_to_json_string(obj);
    *len = strlen(s);
    memcpy(buf, s, *len);
    buf[*len] = 0;

    json_object_put(obj);
}

static int dotbot_stream_callback(struct libwebsocket_context *context,
        struct libwebsocket *wsi,
        enum libwebsocket_callback_reasons reason,
        void *user, void *in, size_t len) {

    int n = 0, m;
    unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 + LWS_SEND_BUFFER_POST_PADDING];
    unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
    struct per_session_data *data = (struct per_session_data*)user;

    switch (reason) {
        /* New connection; initialize a game for them. */
        case LWS_CALLBACK_ESTABLISHED:
            lwsl_info("dotbot_stream_callback: LWS_CALLBACK_ESTABLISHED\n");
            init_session_data(data);
            libwebsocket_callback_on_writable(context, wsi);
            break;
        /* Calculate a move, update the board, and send back the results. */
        case LWS_CALLBACK_SERVER_WRITEABLE:
            tick(&n, (char *)p, data);
            if (n) {
                m = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT);
                if (m < n) {
                    lwsl_info("client disconnected\n");
                    return -1;
                }
            }
            break;
        case LWS_CALLBACK_RECEIVE:
            if (len == 4 && strncmp((const char *)in, "next", len) == 0) {
                libwebsocket_callback_on_writable(context, wsi);
            }
            break;
        default:
            break;
    }

    return 0;
}

void sighandler() {
    force_exit = 1;
    libwebsocket_cancel_service(context);
}

static struct libwebsocket_protocols protocols[] = {
    {
        "dotbot-stream",
        dotbot_stream_callback,
        sizeof(struct per_session_data),
        0, 0, NULL, NULL, 0
    },

    {NULL, NULL, 0, 0, 0, NULL, NULL, 0}
};

int main() {
	struct lws_context_creation_info info;
    char *port;

    signal(SIGINT, sighandler);

    port = getenv("PORT");

	memset(&info, 0, sizeof info);
	info.port = (port == NULL) ? DEFAULT_PORT : atoi(port);
	info.protocols = protocols;
	info.extensions = libwebsocket_get_internal_extensions();
	info.gid = -1;
	info.uid = -1;

    context = libwebsocket_create_context(&info);
    if (context == NULL) {
        lwsl_err("libwebsocket init failed\n");
        return -1;
    }

    while (!force_exit) {
        libwebsocket_service(context, 50);
    }

    libwebsocket_context_destroy(context);
    lwsl_notice("exited cleanly\n");

    return 0;
}
