#ifndef WINDOWS_BOOT_H
#define WINDOWS_BOOT_H

#include <efi.h>
#include "gui.h"

typedef struct {
    EFI_DEVICE_PATH *device_path;
    EFI_DEVICE_PATH *file_path;
    CHAR16 *description;
} windows_boot_entry_t;

EFI_STATUS windows_boot(boot_entry_t *entry, EFI_SYSTEM_TABLE *st);

EFI_STATUS windows_find_bootmgr(CHAR16 *partition_uuid,
                                EFI_DEVICE_PATH **bootmgr_path);

EFI_DEVICE_PATH* windows_make_file_path(EFI_HANDLE handle,
                                        CHAR16 *filename);

EFI_STATUS efi_chainload(EFI_HANDLE image_handle,
                         EFI_SYSTEM_TABLE *system_table,
                         CHAR16 *path,
                         CHAR16 *partition_uuid);

#endif
