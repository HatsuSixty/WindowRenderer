#include <SRMConnector.h>
#include <SRMConnectorMode.h>
#include <SRMCore.h>
#include <SRMDevice.h>
#include <SRMListener.h>

#include <SRMList.h>
#include <SRMLog.h>

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <EGL/egl.h>

#include "application.h"
#include "log.h"
#include "renderer/renderer.h"

bool should_quit = false;

// Set only once. The same resolution is used
// for all monitors.
int screen_width = 0;
int screen_height = 0;

static int open_restricted(const char* path, int flags, void* user_data)
{
    (void)user_data;
    return open(path, flags);
}

static void close_restricted(int fd, void* user_data)
{
    (void)user_data;
    close(fd);
}

static SRMInterface srm_interface = {
    .openRestricted = &open_restricted,
    .closeRestricted = &close_restricted
};

static void initialize_gl(SRMConnector* connector, void* user_data)
{
    (void)user_data;

    SRMConnectorMode* mode = srmConnectorGetCurrentMode(connector);

    if (screen_width == 0)
        screen_width = srmConnectorModeGetWidth(mode);

    if (screen_height == 0)
        screen_height = srmConnectorModeGetHeight(mode);

    Renderer* renderer = renderer_create(screen_width, screen_height);
    application_init_graphics(renderer);
    srmConnectorSetUserData(connector, renderer);

    srmConnectorRepaint(connector);
}

static void paint_gl(SRMConnector* connector, void* user_data)
{
    (void)user_data;

    Renderer* renderer = srmConnectorGetUserData(connector);

    SRMDevice* device = srmConnectorGetDevice(connector);
    EGLDisplay* egl_display = srmDeviceGetEGLDisplay(device);

    application_render(renderer, egl_display);

    srmConnectorRepaint(connector);
}

static void resize_gl(SRMConnector* connector, void* user_data)
{
    (void)user_data;
    srmConnectorRepaint(connector);
}

static void page_flipped(SRMConnector* connector, void* user_data)
{
    (void)connector;
    (void)user_data;
}

static void uninitialize_gl(SRMConnector* connector, void* user_data)
{
    (void)connector;
    (void)user_data;

    Renderer* renderer = srmConnectorGetUserData(connector);
    renderer_destroy(renderer);
}

static SRMConnectorInterface connector_interface = {
    .initializeGL = &initialize_gl,
    .paintGL = &paint_gl,
    .resizeGL = &resize_gl,
    .pageFlipped = &page_flipped,
    .uninitializeGL = &uninitialize_gl,
};

static void connector_plugged_event_handler(SRMListener* listener, SRMConnector* connector)
{
    (void)listener;

    if (!srmConnectorInitialize(connector, &connector_interface, NULL))
        log_log(LOG_ERROR, "Failed to initialize connector %s",
                srmConnectorGetModel(connector));
}

static void connector_unplugged_event_handler(SRMListener* listener, SRMConnector* connector)
{
    (void)listener;
    (void)connector;
}

void ctrl_c(int signo)
{
    (void)signo;
    should_quit = true;
}

int main(int argc, char const** argv)
{
    signal(SIGINT, ctrl_c);

    if (!application_init(argc, argv))
        return 1;

    SRMCore* core = srmCoreCreate(&srm_interface, NULL);
    if (!core) {
        log_log(LOG_ERROR, "Could not initialize SRM core");
        return 1;
    }

    srmCoreAddConnectorPluggedEventListener(core,
                                            &connector_plugged_event_handler,
                                            NULL);
    srmCoreAddConnectorUnpluggedEventListener(core,
                                              &connector_unplugged_event_handler,
                                              NULL);

    SRMListForeach(device_it, srmCoreGetDevices(core))
    {
        SRMDevice* device = srmListItemGetData(device_it);

        SRMListForeach(connector_it, srmDeviceGetConnectors(device))
        {
            SRMConnector* connector = srmListItemGetData(connector_it);

            if (srmConnectorIsConnected(connector)) {
                if (!srmConnectorInitialize(connector, &connector_interface, NULL))
                    log_log(LOG_ERROR, "Failed to initialize connector %s",
                            srmConnectorGetModel(connector));
            }
        }
    }

    while (!should_quit) {
        if (srmCoreProcessMonitor(core, 0) == -1)
            break;
        application_update();
    }

    srmCoreDestroy(core);

    application_terminate();

    return 0;
}
