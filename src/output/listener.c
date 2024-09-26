#include "output.h"

#include <time.h>    // timespec
#include <stdlib.h>  // free

#include <wlr/util/log.h>  // wlr_log

#include "server.h"  // ACNCageServer

/***** Static function declarations *****/

/** Render frame request **/
static int Create_FrameRequest_Listener(struct ACNCageOutput* output);
static void Frame_Request(struct wl_listener* listener, void* data);

/** Output destroy **/
static int Create_OutputDestroy_Listener(struct ACNCageOutput* output);
static void Output_Destroy(struct wl_listener* listener, void* data);

/****************************************/

int ACNCageOutput_CreateListeners(struct ACNCageOutput* output) {
    // Render frame request listener
    if (Create_FrameRequest_Listener(output) != 0) return -1;

    // Output destroy listener
    if (Create_OutputDestroy_Listener(output) != 0) return -1;

    return 0;
}

static int Create_FrameRequest_Listener(struct ACNCageOutput* output) {
    output->frameRequestListener.notify = Frame_Request;
    wl_signal_add(&output->wlr_output->events.frame, &output->frameRequestListener);
    return 0;
}

// Raise by the output, every time it's ready to display a frame
static void Frame_Request(struct wl_listener* listener,
                          void* data __attribute__((unused))) {
    struct ACNCageOutput* output =
        wl_container_of(listener, output, frameRequestListener);

    struct wlr_scene* scene = output->server->scene;

    struct wlr_scene_output* scene_output =
        wlr_scene_get_scene_output(scene, output->wlr_output);
    if (scene_output == NULL) {
        wlr_log(WLR_ERROR, "Output hasn't been added to the scene-graph");
        return;
    }

    // Render the scene and commit this output
    // Note: Multiple optimization techniques are applied under the hood
    if (!wlr_scene_output_commit(scene_output))
        wlr_log(WLR_ERROR, "Failed to Render the scene or to Commit this output");

    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) == -1)
        wlr_log(WLR_ERROR, "%s", strerror(errno));

    // Undefined behavior if clock_gettime failed
    wlr_scene_output_send_frame_done(scene_output, &now);
}

static int Create_OutputDestroy_Listener(struct ACNCageOutput* output) {
    output->outputDestroyListener.notify = Output_Destroy;
    wl_signal_add(&output->wlr_output->events.destroy,
                  &output->outputDestroyListener);
    return 0;
}

// Raise by the output, as part of it's self-destruction process
static void Output_Destroy(struct wl_listener* listener,
                           void* data __attribute__((unused))) {
    struct ACNCageOutput* output =
        wl_container_of(listener, output, outputDestroyListener);

    wl_list_remove(&output->frameRequestListener.link);
    wl_list_remove(&output->outputDestroyListener.link);
    wl_list_remove(&output->link);
    free(output);
}
