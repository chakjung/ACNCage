#include <stdlib.h>  // EXIT_SUCCESS

#include <wlr/util/log.h>  // wlr_log_init, wlr_log

#include "server.h"  // ACNCageServer

int main() {
    // Use default logger
    wlr_log_init(WLR_DEBUG, NULL);

    // Init ACNCageServer
    struct ACNCageServer server = {0};
    if (ACNCageServer_init(&server) != 0) {
        wlr_log(WLR_ERROR, "Failed to init ACNCageServer");
        ACNCageServer_destroy(&server);
        return EXIT_FAILURE;
    }

    // Create interfaces on ACNCageServer
    if (ACNCageServer_CreateInterfaces(&server) != 0) {
        wlr_log(WLR_ERROR, "Failed to create server interfaces");
        ACNCageServer_destroy(&server);
        return EXIT_FAILURE;
    }

    // Create listeners on ACNCageServer
    if (ACNCageServer_CreateListeners(&server) != 0) {
        wlr_log(WLR_ERROR, "Failed to create listeners");
        ACNCageServer_destroy(&server);
        return EXIT_FAILURE;
    }

    ACNCageServer_destroy(&server);
    return EXIT_SUCCESS;
}
