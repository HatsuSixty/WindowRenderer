#include "WRGL/wrgl.h"

#include <dirent.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

bool wrgl_find_gpu_device(char* gpu_device_path, size_t gpu_device_path_len)
{
    char const* dev_dri = "/dev/dri/";

    DIR* directory = opendir(dev_dri);
    if (directory == NULL) {
        log_log(LOG_ERROR, "Failed to open directory `%s`: %s",
                dev_dri, strerror(errno));
        return false;
    }

    struct dirent* directory_entry;

    while ((directory_entry = readdir(directory)) != NULL) {
        if (strncmp(directory_entry->d_name, "renderD", 7) == 0) {
            snprintf(gpu_device_path, gpu_device_path_len,
                     "%s%s", dev_dri, directory_entry->d_name);
            return true;
        }
    }

    while ((directory_entry = readdir(directory)) != NULL) {
        if (strncmp(directory_entry->d_name, "card", 4) == 0) {
            snprintf(gpu_device_path, gpu_device_path_len,
                     "%s%s", dev_dri, directory_entry->d_name);
            return true;
        }
    }

    return false;
}
