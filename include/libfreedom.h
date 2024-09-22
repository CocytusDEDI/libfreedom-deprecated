#ifndef LIBFREEDOM_H
#define LIBFREEDOM_H

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

void free_system_data(SystemData *system_data);
SystemData make_empty_system_data();
SystemDataResult make_system_data_ok_result(SystemData system_data);
SystemDataResult make_system_data_err_result(const char *error);
int is_drm_device(const char *path);
SystemDataResult get_system_data();

typedef struct {
    int os;
    int architecture;
    int firmware;
    int *connectors;
} SystemFreedom;

typedef struct {
    int status;
    SystemFreedom system_freedom;
    const char *error;
} SystemFreedomResult;

void free_system_freedom(SystemFreedom *system_freedom);
SystemFreedom make_empty_system_freedom();
SystemFreedomResult make_system_freedom_ok_result(SystemFreedom system_freedom);
SystemFreedomResult make_system_freedom_err_result(const char *error);
SystemFreedomResult get_system_freedom(SystemData *system_data);

#endif // LIBFREEDOM_H
