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

#include "application.h"

bool should_quit = false;

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
    Application* application = user_data;

    SRMConnectorMode* mode = srmConnectorGetCurrentMode(connector);

    int width = srmConnectorModeGetWidth(mode);
    int height = srmConnectorModeGetHeight(mode);

    if (!application_init_graphics(application, width, height)) {
        should_quit = true;
    }

    srmConnectorRepaint(connector);
}

static void paint_gl(SRMConnector* connector, void* user_data)
{
    Application* application = user_data;

    application_render(application);

    srmConnectorRepaint(connector);
}

static void resize_gl(SRMConnector* connector, void* user_data)
{
    Application* application = user_data;

    SRMConnectorMode* mode = srmConnectorGetCurrentMode(connector);

    int width = srmConnectorModeGetWidth(mode);
    int height = srmConnectorModeGetHeight(mode);

    application_resize(application, width, height);

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

    Application* application = user_data;

    application_destroy_graphics(application);
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

    Application* application = srmListenerGetUserData(listener);

    if (!srmConnectorInitialize(connector, &connector_interface, application))
        fprintf(stderr, "ERROR: failed to initialize connector %s.",
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

int main(void)
{
    signal(SIGINT, ctrl_c);

    Application* application = application_create();
    if (!application)
        return 1;

    SRMCore* core = srmCoreCreate(&srm_interface, NULL);
    srmCoreSetUserData(core, application);

    if (!core) {
        fprintf(stderr, "ERROR: could not initialize SRM core\n");
        return 1;
    }

    SRMListener* connector_plugged_listener = srmCoreAddConnectorPluggedEventListener(core,
                                                                                      &connector_plugged_event_handler,
                                                                                      NULL);
    srmListenerSetUserData(connector_plugged_listener, application);

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
                if (!srmConnectorInitialize(connector, &connector_interface, application))
                    fprintf(stderr, "ERROR: failed to initialize connector %s\n",
                            srmConnectorGetModel(connector));
            }
        }
    }

    while (!should_quit) {
        if (srmCoreProcessMonitor(core, -1) < 0)
            break;
    }

    srmCoreDestroy(core);

    application_destroy(application);

    return 0;
}