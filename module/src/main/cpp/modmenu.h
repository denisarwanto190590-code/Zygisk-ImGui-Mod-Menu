//
// Created by Reveny on 2023/1/13.
//
#define targetLibName OBFUSCATE("libil2cpp.so")

#include <jni.h>
#include <sys/system_properties.h>
#include <unistd.h>

#include "KittyMemory/MemoryPatch.h"
#include "Includes/ESP.h"
#include "Includes/Dobby/dobby.h"
#include "Includes/Utils.h"
#include "Includes/ImGui.h"
#include "modmenu.h" // Menghubungkan ke file menu yang sudah kita buat

static int enable_hack;
static char *game_data_dir = nullptr;

// 1. DIUBAH: Menyesuaikan package name target ke Free Fire sesuai hasil Frida Dumper Anda
static char *game_package_name = "com.dts.freefireth";

// Struktur data Vector3 untuk koordinat game Unity
struct Vector3 {
    float x, y, z;
};

// --- LOGIKA HOOK OFFSET ESP ---

// Pointer fungsi asli get_Position
void *(*old_GetPosition)(void *instance);

// Fungsi Hook untuk mengambil posisi objek/pemain
void *hk_GetPosition(void *instance) {
    if (instance != nullptr) {
        // Di sini tempat ImGui memproses koordinat dari instance untuk digambar ke layar
    }
    return old_GetPosition(instance);
}

// Pointer fungsi asli UGCAPIWorldToScreenPoint (Kamera)
void *(*old_WorldToScreenPoint)(void *camera, Vector3 position, int eye, Vector3 *output);

// Fungsi Hook Kamera untuk konversi koordinat 3D ke 2D Layar
void *hk_WorldToScreenPoint(void *camera, Vector3 position, int eye, Vector3 *output) {
    return old_WorldToScreenPoint(camera, position, eye, output);
}

int isGame(JNIEnv *env, jstring appDataDir) {
    if (!appDataDir) {
        return 0;
    }
    const char *app_data_dir = env->GetStringUTFChars(appDataDir, nullptr);
    static char package_name[256];
    if (sscanf(app_data_dir, OBFUSCATE("/data/%*[^/]/%d/%s"), 0, package_name) != 2) {
        if (sscanf(app_data_dir, OBFUSCATE("/data/%*[^/]/%s"), package_name) != 1) {
            package_name[0] = '\0';
            LOGW(OBFUSCATE("can't parse %s"), app_data_dir);
            return 0;
        }
    }
    if (strcmp(package_name, game_package_name) == 0) {
        LOGI(OBFUSCATE("detect game: %s"), package_name);
        game_data_dir = new char[strlen(app_data_dir) + 1];
        strcpy(game_data_dir, app_data_dir);
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 1;
    } else {
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 0;
    }
}

// Mengarahkan tampilan menu ke DrawMenu() yang ada di modmenu.h
void drawMenu() {
    DrawMenu();
}

void *hack_thread(void *) {
    LOGI(OBFUSCATE("hack thread: %d"), gettid());
    initModMenu((void *)drawMenu);

    // Check if target lib is loaded
    do {
        LOGI(OBFUSCATE("Trying to find lib... PID: %d"), getpid());
        sleep(1);
    } while (!isLibraryLoaded(targetLibName));

    LOGI(OBFUSCATE("%s has been loaded, 0x%lx"), (const char *) targetLibName, getBaseAddress(targetLibName));

    // 2. DIUBAH: Mengambil alamat dasar (Base Address) libil2cpp.so
    unsigned long il2cpp_base = getBaseAddress(targetLibName);

    // 3. DIUBAH: Memasang DobbyHook menggunakan offset yang Anda temukan
    // [HOOK 1] Offset get_Position (0x7CEE8A0)
    dobby_dummy_hook((void *)(il2cpp_base + 0x7CEE8A0), (void *)hk_GetPosition, (void **)&old_GetPosition);

    // [HOOK 2] Offset Kamera UGCAPIWorldToScreenPoint (0x80BFB2C)
    dobby_dummy_hook((void *)(il2cpp_base + 0x80BFB2C), (void *)hk_WorldToScreenPoint, (void **)&old_WorldToScreenPoint);

    LOGI(OBFUSCATE("Done"));
    return nullptr;
}
