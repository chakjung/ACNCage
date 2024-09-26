#include "cursor.h"

#include <wlr/util/log.h>  // wlr_log

#include "server.h"  // ACNCageServer
#include "view.h"    // ACNCageView_focus

/***** Static function declarations *****/

/** Helper functions **/
static struct ACNCageView* Identify_Accessed_View(struct ACNCageServer* server,  //
                                                  double layoutLocalX,
                                                  double layoutLocalY,
                                                  struct wlr_surface** surface,
                                                  double* surfaceLocalX,
                                                  double* surfaceLocalY);
static void Process_Cursor_Motion(struct ACNCageServer* server, uint32_t time_msec);

/** Cursor motion event **/
static int Create_CursorMotion_Listener(struct ACNCageServer* server);
static void Cursor_Motion(struct wl_listener* listener, void* data);

/** Cursor motion event (absolute) **/
static int Create_CursorMotionAbsolute_Listener(struct ACNCageServer* server);
static void Cursor_MotionAbsolute(struct wl_listener* listener, void* data);

/** Cursor button event **/
static int Create_CursorButton_Listener(struct ACNCageServer* server);
static void Cursor_Button(struct wl_listener* listener, void* data);

/** Cursor axis event **/
static int Create_CursorAxis_Listener(struct ACNCageServer* server);
static void Cursor_Axis(struct wl_listener* listener, void* data);

/** Cursor frame event **/
static int Create_CursorFrame_Listener(struct ACNCageServer* server);
static void Cursor_Frame(struct wl_listener* listener, void* data);

/****************************************/

int ACNCageServer_CreateCursorListeners(struct ACNCageServer* server) {
    // Cursor motion event listener
    if (Create_CursorMotion_Listener(server) != 0) return -1;

    // Cursor motion event listener (absolute)
    if (Create_CursorMotionAbsolute_Listener(server) != 0) return -1;

    // Cursor button event listener
    if (Create_CursorButton_Listener(server) != 0) return -1;

    // Cursor axis event listener
    if (Create_CursorAxis_Listener(server) != 0) return -1;

    // Cursor frame event listener
    if (Create_CursorFrame_Listener(server) != 0) return -1;

    return 0;
}

static struct ACNCageView* Identify_Accessed_View(struct ACNCageServer* server,  //
                                                  double layoutLocalX,
                                                  double layoutLocalY,
                                                  struct wlr_surface** surface,
                                                  double* surfaceLocalX,
                                                  double* surfaceLocalY) {
    // Identify the topmost scene node, containing the layout local coordinate
    struct wlr_scene_node* scene_node =
        wlr_scene_node_at(&server->scene->tree.node, layoutLocalX, layoutLocalY,
                          surfaceLocalX, surfaceLocalY);
    if (scene_node == NULL || scene_node->type != WLR_SCENE_NODE_BUFFER) return NULL;

    // Retrieve scene surface from scene node
    struct wlr_scene_surface* scene_surface =
        wlr_scene_surface_from_buffer(wlr_scene_buffer_from_node(scene_node));
    if (scene_surface == NULL) return NULL;
    *surface = scene_surface->surface;

    // Retrieve the ancestor, that contains the ACNCageView
    struct wlr_scene_tree* scene_tree = scene_node->parent;
    while (scene_tree->node.data == NULL) {
        scene_tree = scene_tree->node.parent;
        if (scene_tree == NULL) return NULL;
    }
    return scene_tree->node.data;
}

static void Process_Cursor_Motion(struct ACNCageServer* server, uint32_t time_msec) {
    struct wlr_surface* surface = NULL;
    double surfaceLocalX, surfaceLocalY;
    struct ACNCageView* view =
        Identify_Accessed_View(server, server->cursor->x, server->cursor->y,
                               &surface, &surfaceLocalX, &surfaceLocalY);

    // No view accessed, reset cursor image to default
    if (view == NULL)
        wlr_xcursor_manager_set_cursor_image(server->xcursor_manager, "left_ptr",
                                             server->cursor);

    if (surface == NULL) {
        // Clear pointer focus
        // So that future events, are not sent to the previous client.
        wlr_seat_pointer_clear_focus(server->seat);
    } else {
        // Send pointer enter & motion events
        // The enter event also gives the surface pointer focus
        wlr_seat_pointer_notify_enter(server->seat, surface, surfaceLocalX,
                                      surfaceLocalY);
        wlr_seat_pointer_notify_motion(server->seat, time_msec, surfaceLocalX,
                                       surfaceLocalY);
    }
}

static int Create_CursorMotion_Listener(struct ACNCageServer* server) {
    server->cursorMotionListener.notify = Cursor_Motion;
    wl_signal_add(&server->cursor->events.motion, &server->cursorMotionListener);
    return 0;
}

// Raise by the cursor, when a pointer emits a relative motion event (i.e. a delta)
static void Cursor_Motion(struct wl_listener* listener, void* data) {
    struct ACNCageServer* server =
        wl_container_of(listener, server, cursorMotionListener);
    struct wlr_pointer_motion_event* event = data;

    wlr_cursor_move(server->cursor, &event->pointer->base, event->delta_x,
                    event->delta_y);
    Process_Cursor_Motion(server, event->time_msec);
}

static int Create_CursorMotionAbsolute_Listener(struct ACNCageServer* server) {
    server->cursorMotionAbsoluteListener.notify = Cursor_MotionAbsolute;
    wl_signal_add(&server->cursor->events.motion_absolute,
                  &server->cursorMotionAbsoluteListener);
    return 0;
}

// Raise by the cursor, when a pointer emits an absolute motion event
static void Cursor_MotionAbsolute(struct wl_listener* listener, void* data) {
    struct ACNCageServer* server =
        wl_container_of(listener, server, cursorMotionAbsoluteListener);
    struct wlr_pointer_motion_absolute_event* event = data;

    wlr_cursor_warp_absolute(server->cursor, &event->pointer->base, event->x,
                             event->y);
    Process_Cursor_Motion(server, event->time_msec);
}

static int Create_CursorButton_Listener(struct ACNCageServer* server) {
    server->cursorButtonListener.notify = Cursor_Button;
    wl_signal_add(&server->cursor->events.button, &server->cursorButtonListener);
    return 0;
}

// Raise by the cursor, when a pointer emits a button event
static void Cursor_Button(struct wl_listener* listener, void* data) {
    struct ACNCageServer* server =
        wl_container_of(listener, server, cursorButtonListener);
    struct wlr_pointer_button_event* event = data;

    // Notify the client w. pointer focus, that a button event has occurred
    wlr_seat_pointer_notify_button(server->seat, event->time_msec, event->button,
                                   event->state);

    struct wlr_surface* surface = NULL;
    double surfaceLocalX, surfaceLocalY;
    struct ACNCageView* view =
        Identify_Accessed_View(server, server->cursor->x, server->cursor->y,
                               &surface, &surfaceLocalX, &surfaceLocalY);

    if (view != NULL && event->state != WLR_BUTTON_RELEASED)
        ACNCageView_focus(view, surface);
}

static int Create_CursorAxis_Listener(struct ACNCageServer* server) {
    server->cursorAxisListener.notify = Cursor_Axis;
    wl_signal_add(&server->cursor->events.axis, &server->cursorAxisListener);
    return 0;
}

// Raise by the cursor, when a pointer emits an axis event
static void Cursor_Axis(struct wl_listener* listener, void* data) {
    struct ACNCageServer* server =
        wl_container_of(listener, server, cursorAxisListener);
    struct wlr_pointer_axis_event* event = data;

    // Notify the client w. pointer focus, that an axis event has occurred
    wlr_seat_pointer_notify_axis(server->seat, event->time_msec, event->orientation,
                                 event->delta, event->delta_discrete, event->source);
}

static int Create_CursorFrame_Listener(struct ACNCageServer* server) {
    server->cursorFrameListener.notify = Cursor_Frame;
    wl_signal_add(&server->cursor->events.frame, &server->cursorFrameListener);
    return 0;
}

// Raise by the cursor, when a pointer emits a frame event
static void Cursor_Frame(struct wl_listener* listener, void* data) {
    struct ACNCageServer* server =
        wl_container_of(listener, server, cursorFrameListener);

    // Notify the client w. pointer focus, that a frame event has occurred
    wlr_seat_pointer_notify_frame(server->seat);
}
