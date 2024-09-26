#include "server.h"

#include <wlr/util/log.h>  // wlr_log

#include <wlr/types/wlr_xdg_shell.h>  // wlr_xdg_shell

// Interfaces
#include <wlr/types/wlr_compositor.h>     // wlr_compositor_create
#include <wlr/types/wlr_subcompositor.h>  // wlr_subcompositor_create
#include <wlr/types/wlr_data_device.h>    // wlr_data_device_manager_create

int ACNCageServer_init(struct ACNCageServer* server) {
    if (server == NULL) return -1;

    server->wl_display = wl_display_create();
    if (server->wl_display == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wl_display");
        return -1;
    }

    server->backend = wlr_backend_autocreate(server->wl_display);
    if (server->backend == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wlr_backend");
        return -1;
    }

    server->renderer = wlr_renderer_autocreate(server->backend);
    if (server->renderer == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wlr_renderer");
        return -1;
    }

    if (!wlr_renderer_init_wl_display(server->renderer, server->wl_display)) {
        wlr_log(WLR_ERROR, "Failed to init wlr_renderer");
        return -1;
    }

    // wlr_allocator handles buffer allocation
    // Which then bridges the backend and the renderer
    server->allocator = wlr_allocator_autocreate(server->backend, server->renderer);
    if (server->allocator == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wlr_allocator");
        return -1;
    }

    // Helper to arrange outputs in a 2D coordinate space
    server->output_layout = wlr_output_layout_create();
    if (server->output_layout == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wlr_output_layout");
        return -1;
    }

    // Abstraction that handles all rendering and dmg tracking
    server->scene = wlr_scene_create();
    if (server->scene == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wlr_scene");
        return -1;
    }

    if (!wlr_scene_attach_output_layout(server->scene, server->output_layout)) {
        wlr_log(WLR_ERROR, "Failed to attach output_layout to scene");
        return -1;
    }

    // XDG-shell is a Wayland protocol, used to describe application windows
    server->xdg_shell = wlr_xdg_shell_create(server->wl_display, 3);
    if (server->xdg_shell == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wlr_xdg_shell");
        return -1;
    }

    // wlr_cursor is a wlroots utility, implementing the behavior of a cursor
    server->cursor = wlr_cursor_create();
    if (server->cursor == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wlr_cursor");
        return -1;
    }
    wlr_cursor_attach_output_layout(server->cursor, server->output_layout);

    // wlr_xcursor_manager is a wlroots utility, which loads xcursor themes
    // For the compositor to source cursor images from
    server->xcursor_manager = wlr_xcursor_manager_create(NULL, 24);
    if (server->xcursor_manager == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wlr_xcursor_manager");
        return -1;
    }
    if (!wlr_xcursor_manager_load(server->xcursor_manager, 1)) {
        wlr_log(WLR_ERROR, "Failed to load xcursor themes");
        return -1;
    }

    // wlr_seat is an abstraction on top of wl_seat, which provides an abstraction
    // over input events on Wayland
    server->seat = wlr_seat_create(server->wl_display, "seat0");
    if (server->seat == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wlr_seat");
        return -1;
    }

    return 0;
}

void ACNCageServer_destroy(struct ACNCageServer* server) {
    if (server == NULL) return;

    if (server->seat != NULL) wlr_seat_destroy(server->seat);

    if (server->xcursor_manager != NULL)
        wlr_xcursor_manager_destroy(server->xcursor_manager);

    if (server->cursor != NULL) wlr_cursor_destroy(server->cursor);

    if (server->output_layout != NULL)
        wlr_output_layout_destroy(server->output_layout);

    if (server->allocator != NULL) wlr_allocator_destroy(server->allocator);

    if (server->renderer != NULL) wlr_renderer_destroy(server->renderer);

    if (server->backend != NULL) wlr_backend_destroy(server->backend);

    if (server->wl_display != NULL) wl_display_destroy(server->wl_display);
}

int ACNCageServer_CreateInterfaces(struct ACNCageServer* server) {
    // wlr_compositor provides surface allocation
    if (wlr_compositor_create(server->wl_display, server->renderer) == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wlr_compositor");
        return -1;
    }

    // wlr_subcompositor allows surfaces to be assigned a subsurface role
    if (wlr_subcompositor_create(server->wl_display) == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wlr_subcompositor");
        return -1;
    }

    // wlr_data_device_manager handles the clipboard
    if (wlr_data_device_manager_create(server->wl_display) == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wlr_data_device_manager");
        return -1;
    }

    return 0;
}
