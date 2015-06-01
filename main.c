#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include "main.h"

static int num_connections = 0;

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
            num_connections++;
            break;
        /* Calculate a move, update the board, and send back the results. */
        case LWS_CALLBACK_SERVER_WRITEABLE:
            tick(&n, (char *)p, data);
            if (n) {
                m = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT);
                if (m < n) {
                    lwsl_info("client disconnected\n");
                    num_connections--;
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

static void init_session_data(struct per_session_data *data) {
    data->first = 1;
    data->score = 0;
    memset(data->grid, 0, sizeof(data->grid));
    fill_grid(data->grid, EMPTY);
}

static void fmt_grid(int *len, char **buf, grid_t grid) {
    (*buf)[0] = '[';
    (*buf)[1] = '"';
    *len += 2;
    *buf += 2;
    char *s = grid_to_string(grid);
    int row = 0, i = 0;
    while (row < NUM_ROWS) {
        if (s[i] == '\n') {
            if (++row < NUM_ROWS) {
                (*buf)[0] = '"';
                (*buf)[1] = ',';
                (*buf)[2] = '"';
                *len += 3;
                *buf += 3;
            }
        } else if (s[i] != ' ') {
            (*buf)[0] = s[i];
            *len += 1;
            *buf += 1;
        }
        i++;
    }
    free(s);
    (*buf)[0] = '"';
    (*buf)[1] = ']';
    *len += 2;
    *buf += 2;
}

static void fmt_path(int *len, char **buf, int path_length, path_t path) {
    (*buf)[0] = '[';
    *len += 1;
    *buf += 1;
    int i;
    for (i = 0; i < path_length; i++) {
        int n = sprintf(*buf, "[%d,%d],", INDEX_ROW(path[i]), INDEX_COL(path[i]));
        *len += n;
        *buf += n;
    }
    (*buf)[-1] = ']';
}

static void tick(int *len, char *buf, struct per_session_data *data) {
    int path_length = 0;
    path_t path;
    struct timeval tv;
    long ms;

    if (data->first) {
        data->first = 0;
    } else {
        gettimeofday(&tv, NULL);
        ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
        if ((ms - data->last_updated) < min_update_interval) {
            return;
        }
        data->last_updated = ms;
        int num_moves;
        move_list_t moves;
        get_moves(data->grid, &num_moves, moves);
        /* Naively choose the largest move. */
        int max_size = -1;
        mask_t move = EMPTY_MASK;
        int i;
        for (i = 0; i < num_moves; i++) {
            int size = 0;
            mask_t tmp = moves[i];
            while (tmp) {
                tmp ^= tmp & -tmp;
                size++;
            }
            if (size > max_size) {
                max_size = size;
                move = moves[i];
            }
        }
        apply_mask(data->grid, move);
        fill_grid(data->grid, EMPTY);
        mask_to_path(move, &path_length, path);
    }
    int n = sprintf(buf, "{\"grid\":");
    *len += n;
    buf += n;
    fmt_grid(len, &buf, data->grid);
    if (path_length) {
        n = sprintf(buf, ",\"path\":");
        *len += n;
        buf += n;
        fmt_path(len, &buf, path_length, path);
    }
    buf[0] = '}';
    *len += 1;
    buf += 1;
    buf[0] = 0;
}

void sighandler() {
    force_exit = 1;
    libwebsocket_cancel_service(context);
}

int main() {
    signal(SIGINT, sighandler);

	struct lws_context_creation_info info;
	memset(&info, 0, sizeof info);

    char *port = getenv("PORT");
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
