#include "view.h"

#include <wlr/util/log.h>  // wlr_log

/***** Static function declarations *****/

/** Map surface **/
static int Create_SurfaceMap_Listener(struct ACNCageView* view,
                                      struct wlr_xdg_surface* wlr_xdg_surface);
static void Surface_Map(struct wl_listener* listener, void* data);

/** Unmap surface **/
static int Create_SurfaceUnmap_Listener(struct ACNCageView* view,
                                        struct wlr_xdg_surface* wlr_xdg_surface);
static void Surface_Unmap(struct wl_listener* listener, void* data);

/** Destroy surface **/
static int Create_SurfaceDestroy_Listener(struct ACNCageView* view,
                                          struct wlr_xdg_surface* wlr_xdg_surface);
static void Surface_Destroy(struct wl_listener* listener, void* data);

/** Fullscreen toplevel request **/
static int Create_ToplevelFullscreenRequest_Listener(struct ACNCageView* view);
static void Toplevel_FullscreenRequest(struct wl_listener* listener, void* data);

/****************************************/

int ACNCageView_CreateListeners(struct ACNCageView* view,
                                struct wlr_xdg_surface* wlr_xdg_surface) {
    //  Surface map listener
    if (Create_SurfaceMap_Listener(view, wlr_xdg_surface) != 0) return -1;

    //  Surface unmap listener
    if (Create_SurfaceUnmap_Listener(view, wlr_xdg_surface) != 0) return -1;

    //  Surface destroy listener
    if (Create_SurfaceDestroy_Listener(view, wlr_xdg_surface) != 0) return -1;

    //  Toplevel fullscreen request listener
    if (Create_ToplevelFullscreenRequest_Listener(view) != 0) return -1;

    return 0;
}

static int Create_SurfaceMap_Listener(struct ACNCageView* view,
                                      struct wlr_xdg_surface* wlr_xdg_surface) {
    view->surfaceMapListener.notify = Surface_Map;
    wl_signal_add(&wlr_xdg_surface->events.map, &view->surfaceMapListener);
    return 0;
}

// Raise by the xdg_surface, when the surface is ready to be shown
static void Surface_Map(struct wl_listener* listener, void* data) {
    struct ACNCageView* view = wl_container_of(listener, view, surfaceMapListener);
    wl_list_insert(&view->server->views, &view->link);
    ACNCageView_focus(view, view->wlr_xdg_toplevel->base->surface);
}

static int Create_SurfaceUnmap_Listener(struct ACNCageView* view,
                                        struct wlr_xdg_surface* wlr_xdg_surface) {
    view->surfaceUnmapListener.notify = Surface_Unmap;
    wl_signal_add(&wlr_xdg_surface->events.unmap, &view->surfaceUnmapListener);
    return 0;
}

static void Surface_Unmap(struct wl_listener* listener, void* data) {
    struct ACNCageView* view = wl_container_of(listener, view, surfaceUnmapListener);
    wl_list_remove(&view->link);
}

static int Create_SurfaceDestroy_Listener(struct ACNCageView* view,
                                          struct wlr_xdg_surface* wlr_xdg_surface) {
    view->surfaceDestroyListener.notify = Surface_Destroy;
    wl_signal_add(&wlr_xdg_surface->events.destroy, &view->surfaceDestroyListener);
    return 0;
}

static void Surface_Destroy(struct wl_listener* listener, void* data) {
    struct ACNCageView* view =
        wl_container_of(listener, view, surfaceDestroyListener);

    wl_list_remove(&view->surfaceMapListener.link);
    wl_list_remove(&view->surfaceUnmapListener.link);
    wl_list_remove(&view->surfaceDestroyListener.link);
    wl_list_remove(&view->toplevelFullscreenRequestListener.link);

    free(view);
}

static int Create_ToplevelFullscreenRequest_Listener(struct ACNCageView* view) {
    view->toplevelFullscreenRequestListener.notify = Toplevel_FullscreenRequest;
    wl_signal_add(&view->wlr_xdg_toplevel->events.request_fullscreen,
                  &view->toplevelFullscreenRequestListener);
    return 0;
}

static void Toplevel_FullscreenRequest(struct wl_listener* listener, void* data) {
    struct ACNCageView* view =
        wl_container_of(listener, view, toplevelFullscreenRequestListener);
    wlr_xdg_toplevel_set_fullscreen(view->wlr_xdg_toplevel,
                                    view->wlr_xdg_toplevel->requested.fullscreen);
}
