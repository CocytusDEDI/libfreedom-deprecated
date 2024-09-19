#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/utsname.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <sys/stat.h>

#define DRM_DIR "/dev/dri/"
#define DRM_CARD_PREFIX "card"

typedef struct {
    char os_name[128];         // Operating system name
    char architecture[32];     // CPU architecture
    char firmware_type[64];    // Type of firmware (e.g., BIOS, UEFI, Coreboot)
    size_t connector_count;
    uint32_t *connector_types;
} SystemData;

typedef struct {
    int status;
    SystemData system_data;
    const char *error;
} SystemDataResult;

// Create an empty SystemData
SystemData create_empty_system_data() {
    SystemData empty;
    snprintf(empty.os_name, sizeof(empty.os_name), "%s", "");
    snprintf(empty.architecture, sizeof(empty.architecture), "%s", "");
    snprintf(empty.firmware_type, sizeof(empty.firmware_type), "%s", "");
    empty.connector_count = 0;
    empty.connector_types = NULL;
    return empty;
}

// Create a successful result
SystemDataResult create_success_result(SystemData system_data) {
    SystemDataResult result;
    result.status = 0;  // Standard success code
    result.system_data = system_data;
    result.error = NULL;
    return result;
}

// Create an error result
SystemDataResult create_error_result(const char *error) {
    SystemDataResult result;
    result.status = 1;
    result.system_data = create_empty_system_data();
    result.error = error;
    return result;
}


int is_drm_device(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0; // File does not exist or cannot be accessed
    }
    return S_ISCHR(st.st_mode); // Return true if it's a character device
}

void free_system_data(SystemData *system_data) {
    if (system_data) {
        free(system_data->connector_types);
        system_data->connector_types = NULL;
    }
}

SystemDataResult get_system_data() {
    SystemData system_data = create_empty_system_data();

    // Getting os name and architecture from utsname
    struct utsname hardwareinfo;
    if (uname(&hardwareinfo) == 0) {
        snprintf(system_data.os_name, sizeof(system_data.os_name), "%s", hardwareinfo.sysname);
        snprintf(system_data.architecture, sizeof(system_data.architecture), "%s", hardwareinfo.machine);
    } else {
        return create_error_result("System detection failure: utsname failed");
    }

    // Getting connectors (e.g. DisplayPort, HDMI, VGA) from drm (Direct Rendering Manager)
    DIR *dir;
    struct dirent *entry;
    char path[256];

    dir = opendir(DRM_DIR);
    if (dir == NULL) {
        return create_error_result("System detection failure: utsname failed");
    }

    while ((entry = readdir(dir)) != NULL) {
        // Check if entry is a DRM card
        if (strncmp(entry->d_name, DRM_CARD_PREFIX, strlen(DRM_CARD_PREFIX)) == 0) {
            snprintf(path, sizeof(path), "%s%s", DRM_DIR, entry->d_name);

            if (is_drm_device(path)) {
                // Open and query the DRM device
                int fd = open(path, O_RDWR | O_CLOEXEC);
                if (fd < 0) {
                    continue;
                }

                // Retrieve resources from DRM
                drmModeRes *resources = drmModeGetResources(fd);
                if (resources) {
                    system_data.connector_types = (uint32_t *)malloc(resources->count_connectors * sizeof(uint32_t));
                    if (system_data.connector_types) {
                        // Populate the array
                        for (int i = 0; i < resources->count_connectors; ++i) {
                            drmModeConnector *connector = drmModeGetConnector(fd, resources->connectors[i]);
                            if (connector) {
                                system_data.connector_types[i] = connector->connector_type;
                                drmModeFreeConnector(connector);
                            }
                        }
                    }
                }

                close(fd);
            }
        }
    }

    closedir(dir);

    return create_success_result(system_data);
}
