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
    char firmware[64];    // Type of firmware (e.g., BIOS, UEFI, Coreboot)
    size_t connector_count;
    uint32_t *connector_types;
} SystemData;

typedef struct {
    int status;
    SystemData system_data;
    const char *error;
} SystemDataResult;

void free_system_data(SystemData *system_data) {
    if (system_data) {
        free(system_data->connector_types);
        system_data->connector_types = NULL;
    }
}

/// Create an empty SystemData
SystemData make_empty_system_data() {
    SystemData empty;
    snprintf(empty.os_name, sizeof(empty.os_name), "%s", "");
    snprintf(empty.architecture, sizeof(empty.architecture), "%s", "");
    snprintf(empty.firmware, sizeof(empty.firmware), "%s", "");
    empty.connector_count = 0;
    empty.connector_types = NULL;
    return empty;
}

/// Create a successful SystemDataResult
SystemDataResult make_system_data_ok_result(SystemData system_data) {
    SystemDataResult result;
    result.status = 0;  // Standard success code
    result.system_data = system_data;
    result.error = NULL;
    return result;
}

/// Create an error SystemDataResult
SystemDataResult make_system_data_err_result(const char *error) {
    SystemDataResult result;
    result.status = 1;
    result.system_data = make_empty_system_data();
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

SystemDataResult get_system_data() {
    SystemData system_data = make_empty_system_data();

    // Getting os name and architecture from utsname
    struct utsname hardwareinfo;
    if (uname(&hardwareinfo) == 0) {
        snprintf(system_data.os_name, sizeof(system_data.os_name), "%s", hardwareinfo.sysname);
        snprintf(system_data.architecture, sizeof(system_data.architecture), "%s", hardwareinfo.machine);
    } else {
        return make_system_data_err_result("System detection failure: utsname failed");
    }

    // Getting connectors (e.g. DisplayPort, HDMI, VGA) from drm (Direct Rendering Manager)
    DIR *dir;
    struct dirent *entry;
    char path[256];

    dir = opendir(DRM_DIR);
    if (dir == NULL) {
        return make_system_data_err_result("System detection failure: utsname failed");
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

    return make_system_data_ok_result(system_data);
}

typedef struct {
    int os;
    int architecture;
    int firmware;
    int *connectors;
} SystemFreedom;

typedef struct {
    int status;
    SystemFreedom system_data;
    const char *error;
} SystemFreedomResult;

void free_system_freedom(SystemFreedom *system_freedom) {
    if (system_freedom) {
        free(system_freedom->connectors);
    }
}

/// Create an empty SystemFreedom
SystemFreedom make_empty_system_freedom() {
    SystemFreedom empty;
    empty.os = -1;
    empty.architecture = -1;
    empty.firmware = -1;
    empty.connectors = NULL;
    return empty;
}

/// Create a successful SystemFreedomResult
SystemFreedomResult make_system_freedom_ok_result(SystemFreedom system_freedom) {
    SystemFreedomResult result;
    result.status = 0;  // Standard success code
    result.system_data = system_freedom;
    result.error = NULL;
    return result;
}

/// Create an error SystemFreedomResult
SystemFreedomResult make_system_freedom_err_result(const char *error) {
    SystemFreedomResult result;
    result.status = 1;
    result.system_data = make_empty_system_freedom();
    result.error = error;
    return result;
}

SystemFreedomResult get_system_freedom(SystemData *system_data) {
    SystemFreedom system_freedom = make_empty_system_freedom();

    // Check OS
    if (strcmp(system_data->os_name, "Linux") == 0) {
        system_freedom.os = 1;
    } else if (strcmp(system_data->os_name, "BSD") == 0) {
        system_freedom.os = 1;
    } else if (strcmp(system_data->os_name, "Windows") == 0) {
        system_freedom.os = 0;
    } else if (strcmp(system_data->os_name, "MacOS") == 0) {
        system_freedom.os = 0;
    }

    // Check Architecture
    if (strcmp(system_data->architecture, "RISC-V") == 0) {
        system_freedom.architecture = 1;
    } else if (strcmp(system_data->architecture, "x86") == 0) {
        system_freedom.architecture = 0;
    } else if (strcmp(system_data->architecture, "arm") == 0) {
        system_freedom.architecture = 0;
    }

    // Check Firmware
    if (strcmp(system_data->firmware, "Coreboot") == 0) {
        system_freedom.firmware = 1;
    }

    return make_system_freedom_ok_result(system_freedom);
}
