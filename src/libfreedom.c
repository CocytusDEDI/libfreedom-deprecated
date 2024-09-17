#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>

// Define the SystemFreedom and SystemFreedomResult structs
typedef struct {
    char os_name[128];         // Operating system name
    char architecture[32];     // CPU architecture
    char firmware_type[64];    // Type of firmware (e.g., BIOS, UEFI, Coreboot)
    int proprietary_drivers;   // Number of proprietary drivers
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
    return empty;
}

// Create an error result
SystemFreedomResult create_error_result(const char *error, int error_code) {
    SystemFreedomResult result;
    result.status = error_code;
    result.system_freedom = create_empty_system_freedom();
    result.error = error;
    return result;
}

SystemFreedomResult get_hardware_freedom() {
    SystemFreedom system_freedom = create_empty_system_freedom();

    struct utsname hardwareinfo;

    if (uname(&hardwareinfo) == 0) {
        snprintf(system_freedom.os_name, sizeof(system_freedom.os_name), "%s", hardwareinfo.sysname);
        snprintf(system_freedom.architecture, sizeof(system_freedom.architecture), "%s", hardwareinfo.machine);
    } else {
        return create_error_result("Hardware detection failure: utsname failed", 1);
    }

    return create_success_result(system_freedom);
}
