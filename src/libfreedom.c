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
    int proprietary_drivers;   // Number of proprietary drivers
    size_t connector_count;
    uint32_t *connector_types;
} SystemFreedom;

typedef struct {
    int status;
    SystemFreedom system_freedom;
    const char *error;
} SystemFreedomResult;

// Create a successful result
SystemFreedomResult create_success_result(SystemFreedom system_freedom) {
    SystemFreedomResult result;
    result.status = 0;  // Standard success code
    result.system_freedom = system_freedom;
    result.error = NULL;
    return result;
}

// Create an empty SystemFreedom
SystemFreedom create_empty_system_freedom() {
    SystemFreedom empty;
    snprintf(empty.os_name, sizeof(empty.os_name), "%s", "");
    snprintf(empty.architecture, sizeof(empty.architecture), "%s", "");
    snprintf(empty.firmware_type, sizeof(empty.firmware_type), "%s", "");
    empty.proprietary_drivers = 0;
    empty.connector_count = 0;
    empty.connector_types = NULL;
    return empty;
}

// Create an error result
SystemFreedomResult create_error_result(const char *error) {
    SystemFreedomResult result;
    result.status = 1;
    result.system_freedom = create_empty_system_freedom();
    result.error = error;
    return result;
}


int is_drm_device(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;  // File does not exist or cannot be accessed
    }
    return S_ISCHR(st.st_mode);  // Return true if it's a character device
}

void free_system_freedom(SystemFreedom *system_freedom) {

}

SystemFreedomResult get_hardware_freedom() {
    SystemFreedom system_freedom = create_empty_system_freedom();

    // Getting os name and architecture from utsname
    struct utsname hardwareinfo;
    if (uname(&hardwareinfo) == 0) {
        snprintf(system_freedom.os_name, sizeof(system_freedom.os_name), "%s", hardwareinfo.sysname);
        snprintf(system_freedom.architecture, sizeof(system_freedom.architecture), "%s", hardwareinfo.machine);
    } else {
        return create_error_result("Hardware detection failure: utsname failed");
    }

    // Getting connectors (e.g. DisplayPort, HDMI, VGA) from drm (Direct Rendering Manager)
    DIR *dir;
    struct dirent *entry;
    char path[256];

    dir = opendir(DRM_DIR);
    if (dir == NULL) {
        create_error_result("Hardware detection failure: utsname failed");
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
                    system_freedom.connector_types = (uint32_t *)malloc(resources->count_connectors * sizeof(uint32_t));
                    if (system_freedom.connector_types) {
                        // Populate the array
                        for (int i = 0; i < resources->count_connectors; ++i) {
                            drmModeConnector *connector = drmModeGetConnector(fd, resources->connectors[i]);
                            if (connector) {
                                system_freedom.connector_types[i] = connector->connector_type;
                                drmModeFreeConnector(connector);
                            } else {
                                system_freedom.connector_types[i] = -1; // Handle error
                            }
                        }
                    }
                }

                close(fd);
            }
        }
    }

    closedir(dir);

    return create_success_result(system_freedom);
}
