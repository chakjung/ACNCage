#pragma once

#include <wayland-server-core.h>  // wl_display

#include <wlr/backend.h>                  // wlr_backend
#include <wlr/render/wlr_renderer.h>      // wlr_renderer
#include <wlr/render/allocator.h>         // wlr_allocator
#include <wlr/types/wlr_output_layout.h>  // wlr_output_layout
#include <wlr/types/wlr_scene.h>          // wlr_scene

struct ACNCageServer {
    // Resources
    struct wl_display* wl_display;
    struct wlr_backend* backend;
    struct wlr_renderer* renderer;
    struct wlr_allocator* allocator;
    struct wlr_output_layout* output_layout;
    struct wlr_scene* scene;

    // Outputs
    struct wl_list outputs;
    struct wl_listener newOutputListener;

    // Shells
    struct wl_list views;
    struct wlr_xdg_shell* xdg_shell;
    struct wl_listener newXdgSurfaceListener;

    // Cursor
    struct wlr_cursor* cursor;
    struct wlr_xcursor_manager* xcursor_manager;
    struct wl_listener cursorMotionListener;
    struct wl_listener cursorMotionAbsoluteListener;
    struct wl_listener cursorButtonListener;
    struct wl_listener cursorAxisListener;
    struct wl_listener cursorFrameListener;

    // Keyboard
    struct wl_list keyboards;

    // Seat
    struct wlr_seat* seat;
    struct wl_listener newInputListener;
};

/**
 * Inits the provided ACNCageServer
 * :param server: server to init
 * :return: Success 0, Error -1
 */
int ACNCageServer_init(struct ACNCageServer* server);

/**
 * Destroy the provided ACNCageServer, and cleanup all its resources
 * :param server: server to destroy
 */
void ACNCageServer_destroy(struct ACNCageServer* server);

/**
 * Create interfaces that clients can interact with
 * :param server: server hosting the interfaces
 * :return: Success 0, Error -1
 */
int ACNCageServer_CreateInterfaces(struct ACNCageServer* server);

/**
 * Create listeners for backend events
 * :param server: server hosting the listeners
 * :return: Success 0, Error -1
 */
int ACNCageServer_CreateListeners(struct ACNCageServer* server);
