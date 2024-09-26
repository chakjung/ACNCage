#pragma once

#include <wlr/types/wlr_xdg_shell.h>  // wlr_xdg_toplevel

struct ACNCageView {
    struct wlr_xdg_toplevel* wlr_xdg_toplevel;
    struct wl_list link;

    struct ACNCageServer* server;
    struct wlr_scene_tree* wlr_scene_tree;

    // Listeners
    struct wl_listener surfaceMapListener;
    struct wl_listener surfaceUnmapListener;
    struct wl_listener surfaceDestroyListener;
    struct wl_listener toplevelFullscreenRequestListener;
};

/**
 * Switch focus to the provided ACNCageView
 * :param    view: view to focus on
 * :param surface: surface associate with this view
 */
void ACNCageView_focus(struct ACNCageView* view, struct wlr_surface* surface);

/**
 * Create listeners for backend events
 * :param view: view hosting the listeners
 * :return: Success 0, Error -1
 */
int ACNCageView_CreateListeners(struct ACNCageView* view,
                                struct wlr_xdg_surface* wlr_xdg_surface);
