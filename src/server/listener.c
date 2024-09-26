#include "server.h"

#include <stdlib.h>  // calloc

#include <wlr/util/log.h>  // wlr_log

#include <wlr/types/wlr_xdg_shell.h>  // wlr_xdg_shell

#include "output.h"  // ACNCageOutput

#include "view.h"  // ACNCageView

#include "keyboard.h"  // ACNCageKeyboard

/***** Static function declarations *****/

/** Outputs **/
static int Create_NewOutput_Listener(struct ACNCageServer *server);
static void New_Output(struct wl_listener *listener, void *data);

/** Shells **/
static int Create_NewXdgSurface_Listener(struct ACNCageServer *server);
static void New_XdgSurface(struct wl_listener *listener, void *data);

/** Inputs **/
static int Create_NewInput_Listener(struct ACNCageServer *server);
static void New_Input(struct wl_listener *listener, void *data);

/****************************************/

int ACNCageServer_CreateListeners(struct ACNCageServer *server) {
    // Outputs listeners
    wl_list_init(&server->outputs);
    if (Create_NewOutput_Listener(server) != 0) return -1;

    // Shells listeners
    wl_list_init(&server->views);
    if (Create_NewXdgSurface_Listener(server) != 0) return -1;

    // Inputs listeners
    wl_list_init(&server->keyboards);
    if (Create_NewInput_Listener(server) != 0) return -1;

    return 0;
}

static int Create_NewOutput_Listener(struct ACNCageServer *server) {
    server->newOutputListener.notify = New_Output;
    wl_signal_add(&server->backend->events.new_output, &server->newOutputListener);
    return 0;
}

// Raise by the backend, when a new output becomes available
static void New_Output(struct wl_listener *listener, void *data) {
    struct wlr_output *wlr_output = data;  // Cast data to wlr_output

    // Get the server hosting this newOutputListener
    struct ACNCageServer *server =
        wl_container_of(listener, server, newOutputListener);

    // Configures the new output to use the server's allocator and renderer
    if (!wlr_output_init_render(wlr_output, server->allocator, server->renderer)) {
        wlr_log(WLR_ERROR, "Failed to init render on wlr_output");
        return;
    }

    /**
     * Select a output mode for the new output (width x height @ refresh rate)
     * Note: Some backends don't have modes!
     */
    if (!wl_list_empty(&wlr_output->modes)) {
        /**
         * Picking the advertised preferred mode
         * If there is no preference, the first mode is picked
         */
        struct wlr_output_mode *preferredMode =
            wlr_output_preferred_mode(wlr_output);
        wlr_log(WLR_INFO, "New wlr_output preferred mode: %d x %d @ %d",
                preferredMode->width, preferredMode->height, preferredMode->refresh);

        // Modify wlr_output pending state
        wlr_output_set_mode(wlr_output, preferredMode);  // Set wlr_output_mode
        wlr_output_enable(wlr_output, true);             // Enable wlr_output

        // Commit wlr_output pending state
        if (!wlr_output_commit(wlr_output)) {
            wlr_log(WLR_ERROR, "Failed to commit wlr_output pending state");
            return;
        }
    }

    // Allocates and initializes a container for the new output
    struct ACNCageOutput *output = calloc(1, sizeof(struct ACNCageOutput));
    if (output == NULL) {
        wlr_log(WLR_ERROR, "Failed to allocate ACNCageOutput");
        return;
    }
    output->wlr_output = wlr_output;
    output->server = server;

    // Create listeners on ACNCageOutput
    if (ACNCageOutput_CreateListeners(output) != 0) {
        wlr_log(WLR_ERROR, "Failed to create listeners");
        free(output);
        return;
    }

    // Register the new output to the server
    wl_list_insert(&server->outputs, &output->link);

    /**
     * Add the new output to the output layout
     * Note: Add auto arranges outputs from left-to-right in the order they appear
     */
    wlr_output_layout_add_auto(server->output_layout, wlr_output);
}

static int Create_NewXdgSurface_Listener(struct ACNCageServer *server) {
    server->newXdgSurfaceListener.notify = New_XdgSurface;
    wl_signal_add(&server->xdg_shell->events.new_surface,
                  &server->newXdgSurfaceListener);
    return 0;
}

// Raise by the xdg_shell, when client submits a new xdg surface
static void New_XdgSurface(struct wl_listener *listener, void *data) {
    struct wlr_xdg_surface *wlr_xdg_surface = data;  // Cast data to wlr_xdg_surface
    if (wlr_xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL &&
        wlr_xdg_surface->role != WLR_XDG_SURFACE_ROLE_POPUP) {
        wlr_log(WLR_ERROR, "Invalid XDG surface role");
        return;
    }

    // Attach the XDG popup to it's parent's scene tree,
    // so that the popup gets rendered
    if (wlr_xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
        struct wlr_xdg_surface *parent =
            wlr_xdg_surface_from_wlr_surface(wlr_xdg_surface->popup->parent);
        struct wlr_scene_tree *parentSceneTree = parent->data;

        wlr_xdg_surface->data =
            wlr_scene_xdg_surface_create(parentSceneTree, wlr_xdg_surface);
        if (wlr_xdg_surface->data == NULL)
            wlr_log(WLR_ERROR, "Failed to attach popup to parent scene tree");

        return;
    }

    // Get the server hosting this newXdgSurfaceListener
    struct ACNCageServer *server =
        wl_container_of(listener, server, newXdgSurfaceListener);

    // Allocates and initializes a container for the new surface
    struct ACNCageView *view = calloc(1, sizeof(struct ACNCageView));
    if (view == NULL) {
        wlr_log(WLR_ERROR, "Failed to allocate ACNCageView");
        return;
    }
    view->wlr_xdg_toplevel = wlr_xdg_surface->toplevel;
    view->server = server;

    // Attach the XDG toplevel to server scene tree,
    // so that the toplevel gets rendered
    view->wlr_scene_tree = wlr_scene_xdg_surface_create(
        &server->scene->tree, view->wlr_xdg_toplevel->base);
    if (view->wlr_scene_tree == NULL) {
        wlr_log(WLR_ERROR, "Failed to attach toplevel to server scene tree");
        free(view);
        return;
    }
    view->wlr_scene_tree->node.data = view;
    wlr_xdg_surface->data = view->wlr_scene_tree;

    // Create listeners on ACNCageView
    if (ACNCageView_CreateListeners(view, wlr_xdg_surface) != 0) {
        wlr_log(WLR_ERROR, "Failed to create listeners");
        free(view);
        return;
    }
}

static int Create_NewInput_Listener(struct ACNCageServer *server) {
    server->newInputListener.notify = New_Input;
    wl_signal_add(&server->backend->events.new_input, &server->newInputListener);
    return 0;
}

// Raise by the backend, when a new input device becomes available
static void New_Input(struct wl_listener *listener, void *data) {
    struct wlr_input_device *wlr_input_device =
        data;  // Cast data to wlr_input_device

    // Get the server hosting this newInputListener
    struct ACNCageServer *server =
        wl_container_of(listener, server, newInputListener);

    switch (wlr_input_device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            New_Keyboard(server, wlr_input_device);
            break;

        default:
            wlr_log(WLR_INFO, "Unknown input device type");
            break;
    }
}

static void New_Keyboard(struct ACNCageServer *server,
                         struct wlr_input_device *device) {
    struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);
    wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

    // Allocates and initializes a container for the new keyboard
    struct ACNCageKeyboard *keyboard = calloc(1, sizeof(struct ACNCageKeyboard));
    if (keyboard == NULL) {
        wlr_log(WLR_ERROR, "Failed to allocate ACNCageKeyboard");
        return;
    }
    keyboard->wlr_keyboard = wlr_keyboard;
    keyboard->server = server;

    // Prepare and assign a keymap to the keyboard
    struct xkb_context *xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (xkb_context == NULL) {
        wlr_log(WLR_ERROR, "Failed to create xkb_context");
        free(keyboard);
        xkb_context_unref(xkb_context);
        return;
    }

    struct xkb_keymap *xkb_keymap =
        xkb_keymap_new_from_names(xkb_context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (xkb_keymap == NULL) {
        wlr_log(WLR_ERROR, "Failed to create xkb_keymap");
        free(keyboard);
        xkb_context_unref(xkb_context);
        xkb_keymap_unref(xkb_keymap);
        return;
    }

    if (!wlr_keyboard_set_keymap(wlr_keyboard, xkb_keymap)) {
        wlr_log(WLR_ERROR, "Failed to assign xkb_keymap to wlr_keyboard");
        free(keyboard);
        xkb_context_unref(xkb_context);
        xkb_keymap_unref(xkb_keymap);
        return;
    }

    xkb_context_unref(xkb_context);
    xkb_keymap_unref(xkb_keymap);

    // Create listeners on ACNCageKeyboard
    if (ACNCageKeyboard_CreateListeners(keyboard, device) != 0) {
        wlr_log(WLR_ERROR, "Failed to create listeners");
        free(keyboard);
        return;
    }

    // Register the new keyboard to the server
    wl_list_insert(&server->keyboards, &keyboard->link);

    // Set this keyboard as the active keyboard for the seat
    wlr_seat_set_keyboard(server->seat, wlr_keyboard);
}
