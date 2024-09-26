#pragma once

#include <wlr/types/wlr_keyboard.h>  // wlr_keyboard

struct ACNCageKeyboard {
    struct wlr_keyboard* wlr_keyboard;
    struct wl_list link;

    struct ACNCageServer* server;

    // Listeners
    struct wl_listener keyboardModifiersListener;
    struct wl_listener keyboardKeyListener;
    struct wl_listener deviceDestroyListener;
};

/**
 * Create listeners for keyboard events
 * :param keyboard: keyboard hosting the listeners
 * :return: Success 0, Error -1
 */
int ACNCageKeyboard_CreateListeners(struct ACNCageKeyboard* keyboard,
                                    struct wlr_input_device* device);
