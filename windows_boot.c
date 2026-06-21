
#include "windows_boot.h"
#include "efi_helpers.h"
#include <efi.h>
#include <efilib.h>

extern EFI_BOOT_SERVICES *BS;
extern EFI_SYSTEM_TABLE *ST;
extern EFI_HANDLE IH;

EFI_DEVICE_PATH* windows_make_file_path(EFI_HANDLE handle, CHAR16 *filename) {
    return efi_make_file_path(handle, filename);
}

EFI_STATUS windows_find_bootmgr(CHAR16 *partition_uuid, EFI_DEVICE_PATH **bootmgr_path) {
    CHAR16 *paths[] = {
        L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi",
        L"\\EFI\\BOOT\\bootmgfw.efi",
        L"\\EFI\\BOOT\\BOOTX64.EFI",
        NULL
    };

    for (int i = 0; paths[i]; i++) {
        *bootmgr_path = efi_file_device_path(paths[i], partition_uuid);
        if (*bootmgr_path) return EFI_SUCCESS;
    }

    return EFI_NOT_FOUND;
}

EFI_STATUS efi_chainload(EFI_HANDLE image_handle,
                          EFI_SYSTEM_TABLE *system_table,
                          CHAR16 *path,
                          CHAR16 *partition_uuid) {
    (void)system_table;

    efi_log(L"win: chainloading via device path");
    efi_log(path);
    efi_print(L"Chainloading: ");
    efi_print(path);
    efi_print(L"\r\n");

    EFI_DEVICE_PATH *dp = efi_file_device_path(path, partition_uuid);

    if (!dp) {
        efi_log(L"ERROR: EFI binary not found on any volume");
        efi_print(L"Failed to open file\r\n");
        return EFI_NOT_FOUND;
    }

    efi_log(L"win: LoadImage() from device path");
    EFI_HANDLE new_handle;
    EFI_STATUS status = BS->LoadImage(FALSE, image_handle, dp, NULL, 0, &new_handle);
    efi_free_pool(dp);

    if (EFI_ERROR(status)) {
        efi_log(L"ERROR: LoadImage failed for the EFI binary");
        efi_print(L"LoadImage failed\r\n");
        return status;
    }

    efi_log(L"win: StartImage() - handing control to Windows Boot Manager");
    status = BS->StartImage(new_handle, NULL, NULL);
    if (EFI_ERROR(status)) {
        efi_log(L"ERROR: StartImage returned - chainload failed");
        efi_print(L"StartImage failed\r\n");
    }
    return status;
}

EFI_STATUS windows_boot(boot_entry_t *entry, EFI_SYSTEM_TABLE *st) {
    efi_log(L"win: booting Windows");
    efi_print(L"Booting Windows\r\n");

    if (entry->kernel_path) {
        return efi_chainload(IH, st, entry->kernel_path, entry->uuid);
    }

    efi_log(L"win: no explicit path, searching for bootmgfw.efi");
    EFI_DEVICE_PATH *bootmgr_path = NULL;
    EFI_STATUS status = windows_find_bootmgr(entry->uuid, &bootmgr_path);
    if (EFI_ERROR(status)) {
        efi_log(L"ERROR: Windows Boot Manager (bootmgfw.efi) not found");
        efi_print(L"Windows Boot Manager not found\r\n");
        return status;
    }

    EFI_HANDLE new_handle;
    status = BS->LoadImage(FALSE, IH, bootmgr_path, NULL, 0, &new_handle);
    if (EFI_ERROR(status)) {
        efi_log(L"ERROR: LoadImage failed for bootmgfw.efi");
        efi_print(L"LoadImage failed for bootmgfw.efi\r\n");
        return status;
    }

    status = BS->StartImage(new_handle, NULL, NULL);
    if (EFI_ERROR(status)) {
        efi_print(L"StartImage failed\r\n");
    }
    return status;
}
