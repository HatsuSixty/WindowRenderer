#include <SRMCore.h>
#include <SRMDevice.h>
#include <SRMConnector.h>
#include <SRMConnectorMode.h>
#include <SRMListener.h>

#include <SRMList.h>
#include <SRMLog.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "application.h"

float color = 0.f;

/* Opens a DRM device */
static int open_restricted(const char *path, int flags, void *userData)
{
    SRM_UNUSED(userData);

    // Here something like libseat could be used instead
    return open(path, flags);
}

/* Closes a DRM device */
static void close_restricted(int fd, void *userData)
{
    SRM_UNUSED(userData);
    close(fd);
}

static SRMInterface srm_interface = {
    .openRestricted = &open_restricted,
    .closeRestricted = &close_restricted
};

static void initialize_gl(SRMConnector *connector, void *user_data)
{
    Application* application = user_data;

    /* You must not do any drawing here as it won't make it to
     * the screen. */

    SRMConnectorMode *mode = srmConnectorGetCurrentMode(connector);

    glViewport(0, 
               0, 
               srmConnectorModeGetWidth(mode), 
               srmConnectorModeGetHeight(mode));

    application_init_graphics(application);

    srmConnectorRepaint(connector);
}

static void paint_gl(SRMConnector *connector, void *user_data)
{
    Application* application = user_data;

    application_render(application);

    srmConnectorRepaint(connector);
}

static void resize_gl(SRMConnector *connector, void *user_data)
{
    (void)user_data;
    
    /* You must not do any drawing here as it won't make it to
     * the screen.
     * This is called when the connector changes its current mode,
     * set with srmConnectorSetMode() */

    SRMConnectorMode *mode = srmConnectorGetCurrentMode(connector);

    glViewport(0, 
               0, 
               srmConnectorModeGetWidth(mode), 
               srmConnectorModeGetHeight(mode));

    srmConnectorRepaint(connector);
}

static void page_flipped(SRMConnector *connector, void *userData)
{
    SRM_UNUSED(connector);
    SRM_UNUSED(userData);

    /* You must not do any drawing here as it won't make it to
     * the screen.
     * This is called when the last rendered frame is now being
     * displayed on screen.
     * Google v-sync for more info. */
}

static void uninitialize_gl(SRMConnector *connector, void *user_data)
{
    Application* application = user_data;
    
    SRM_UNUSED(connector);

    /* You must not do any drawing here as it won't make it to
     * the screen.
     * Here you should free any resource created on initializeGL()
     * like shaders, programs, textures, etc. */

    application_destroy_graphics(application);
}

static SRMConnectorInterface connector_interface = {
    .initializeGL = &initialize_gl,
    .paintGL = &paint_gl,
    .resizeGL = &resize_gl,
    .pageFlipped = &page_flipped,
    .uninitializeGL = &uninitialize_gl,
};

static void connector_plugged_event_handler(SRMListener *listener, SRMConnector *connector)
{
    SRM_UNUSED(listener);

    Application* application = srmListenerGetUserData(listener);

    /* This is called when a new connector is avaliable (E.g. Plugging an HDMI display). */

    if (!srmConnectorInitialize(connector, &connector_interface, application))
        fprintf(stderr, "ERROR: failed to initialize connector %s.",
                srmConnectorGetModel(connector));
}

static void connector_unplugged_event_handler(SRMListener *listener, SRMConnector *connector)
{
    SRM_UNUSED(listener);
    SRM_UNUSED(connector);

    /* This is called when a connector is no longer avaliable (E.g. Unplugging an HDMI display). */

    /* The connnector is automatically uninitialized after this event (if initialized)
     * so calling srmConnectorUninitialize() here is not required. */
}

int main(void)
{
    Application* application = application_create();
    
    SRMCore *core = srmCoreCreate(&srm_interface, NULL);
    srmCoreSetUserData(core, application);

    if (!core) {
        fprintf(stderr, "ERROR: could not initialize SRM core\n");
        return 1;
    }

    SRMListener* connector_plugged_listener =
        srmCoreAddConnectorPluggedEventListener(core,
                                                &connector_plugged_event_handler, 
                                                NULL);
    srmListenerSetUserData(connector_plugged_listener, application);

    srmCoreAddConnectorUnpluggedEventListener(core, 
                                                &connector_unplugged_event_handler,
                                                NULL);

    SRMListForeach(device_it, srmCoreGetDevices(core))
    {
        SRMDevice *device = srmListItemGetData(device_it);

        SRMListForeach(connector_it, srmDeviceGetConnectors(device))
        {
            SRMConnector *connector = srmListItemGetData(connector_it);

            if (srmConnectorIsConnected(connector)) {
                if (!srmConnectorInitialize(connector, &connector_interface, application))
                    fprintf(stderr, "ERROR: failed to initialize connector %s\n",
                            srmConnectorGetModel(connector));
            }
        }
    }

    while (1) {
        if (srmCoreProcessMonitor(core, -1) < 0)
            break;
    }

    srmCoreDestroy(core);

    application_destroy(application);

    return 0;
}