#include <stdio.h>
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

    int n, m;
    unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 1024 + LWS_SEND_BUFFER_POST_PADDING];
    unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
    struct per_session_data *data = (struct per_session_data*)user;

    switch (reason) {
        /* New connection; initialize a game for them. */
        case LWS_CALLBACK_ESTABLISHED:
            lwsl_info("dotbot_stream_callback: LWS_CALLBACK_ESTABLISHED\n");
            init_session_data(data);
            num_connections++;
            break;
        /* Calculate a move, update the board, and send back the results. */
        case LWS_CALLBACK_SERVER_WRITEABLE:
            tick(&n, (char *)p, data);
            m = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT);
            if (m < n) {
                lwsl_info("client disconnected\n");
                num_connections--;
                return -1;
            }
            break;
        /* If they send back "reset", restart the game. */
        case LWS_CALLBACK_RECEIVE:
            if (len >= 5 && strncmp((const char *)in, "reset", len) == 0) {
                init_session_data(data);
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

static void tick(int *len, char *buf, struct per_session_data *data) {
    if (data->first) {
        data->first = 0;
    } else {
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
    }

    char *s = grid_to_string(data->grid);
    *len = sprintf(buf, "%s", s);
    free(s);
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

    unsigned int ms, oldms = 0;

    int n = 0;
    while (n >= 0 && !force_exit) {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
        if ((ms - oldms) > delay_ms) {
            libwebsocket_callback_on_writable_all_protocol(&protocols[0]);
            oldms = ms;
        }

        libwebsocket_service(context, 50);
        printf("Num connections: %d\n", num_connections);
    }

    libwebsocket_context_destroy(context);
    lwsl_notice("exited cleanly\n");

    return 0;
}
