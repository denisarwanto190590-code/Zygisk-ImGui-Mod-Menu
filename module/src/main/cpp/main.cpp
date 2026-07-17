//
// Created by Reveny on 2023/1/13.
//

#include "zygisk.hpp"  // <--- SUDAH DIGANTI AGAR SESUAI DENGAN FOLDER ANDA
#include <unistd.h>
#include <pthread.h>
#include <dobby.h> 
#include "modmenu.h"

// Makro registrasi modul bawaan template AndnixSH Zygisk
REGISTER_ZYGISK_MODULE(ZenithModule)
REGISTER_ZYGISK_COMPANION(ZenithCompanion)

// Struktur data Vector3 untuk koordinat game Unity
struct Vector3 {
    float x, y, z;
};

// --- LOGIKA HOOK OFFSET ESP ---

// 1. Pointer fungsi asli get_Position
Vector3 (*old_GetPosition)(void *instance);

// Fungsi Hook untuk mengambil posisi objek/pemain
Vector3 hk_GetPosition(void *instance) {
    if (instance != nullptr) {
        // Tempat ImGui memproses koordinat dari instance
    }
    return old_GetPosition(instance);
}

// 2. Pointer fungsi asli UGCAPIWorldToScreenPoint (Kamera)
void *(*old_WorldToScreenPoint)(void *camera, Vector3 position, int eye, Vector3 *output);

// Fungsi Hook Kamera untuk konversi koordinat 3D ke 2D Layar
void *hk_WorldToScreenPoint(void *camera, Vector3 position, int eye, Vector3 *output) {
    return old_WorldToScreenPoint(camera, position, eye, output);
}

// --- MAIN HACK THREAD ---
void *hack_thread(void *) {
    uintptr_t il2cpp_base = 0;
    
    // Tunggu sampai library Free Fire termuat di memori
    while (!il2cpp_base) {
        il2cpp_base = get_module_base("libil2cpp.so");
        sleep(1);
    }

    // [HOOK 1] Mengunci offset get_Position (0x80F07A0)
    DobbyHook((void *)(il2cpp_base + 0x80F07A0), (void *)hk_GetPosition, (void **)&old_GetPosition);

    // [HOOK 2] Mengunci offset Kamera UGCAPIWorldToScreenPoint (0x80BFB2C)
    DobbyHook((void *)(il2cpp_base + 0x80BFB2C), (void *)hk_WorldToScreenPoint, (void **)&old_WorldToScreenPoint);

    return nullptr;
}

// --- MANAGEMENT MODULE ---
void ZenithModule::onFieldsLoaded(JNIEnv *env, jstring appDataDir) {
    if (isGame(env, appDataDir)) {
        enable_hack = 1;
    }
}

void ZenithModule::onModuleLoaded() {
    if (enable_hack) {
        pthread_t t;
        pthread_create(&t, nullptr, hack_thread, nullptr);
    }
}
