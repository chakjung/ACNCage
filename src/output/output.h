#pragma once

#include <wlr/types/wlr_output.h>  // wlr_output

struct ACNCageOutput {
    struct wlr_output* wlr_output;
    struct wl_list link;

    struct ACNCageServer* server;

    // Listeners
    struct wl_listener frameRequestListener;
    struct wl_listener outputDestroyListener;
};

/**
 * Create listeners for backend events
 * :param output: output hosting the listeners
 * :return: Success 0, Error -1
 */
int ACNCageOutput_CreateListeners(struct ACNCageOutput* output);
