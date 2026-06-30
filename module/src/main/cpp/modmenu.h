//
// Created by Reveny on 2023/1/13.
// Updated for Free Fire Max ESP Line (Pure Standard C++)
//
#define targetLibName "libil2cpp.so"

#include <jni.h>
#include <sys/system_properties.h>
#include <cmath>
#include <android/log.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "KittyMemory/MemoryPatch.h"
#include "Includes/ESP.h"
#include "Includes/Dobby/dobby.h"
#include "Includes/Utils.h"
#include "Includes/ImGui.h"

// Makro Log Android Standar agar tidak memicu eror "undefined"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "ZygiskMenu", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, "ZygiskMenu", __VA_ARGS__)

// ====================================================================
// DAFTAR OFFSET MATANG ESP LINE (FREE FIRE MAX)
// ====================================================================
#define OFFSET_GET_MAIN                 0xa7ed6c0
#define OFFSET_WORLD_TO_SCREEN          0xa7ed344
#define OFFSET_GET_PLAYER_COUNT         0x645d5c4
#define OFFSET_GET_LOCAL_PLAYER         0x64cbde8
#define OFFSET_GET_PLAYER_BY_INDEX      0x7d3fb8c
#define OFFSET_GET_POSITION             0x8857b00
#define OFFSET_IS_DEAD                  0x76611dc

// Struktur data matematika standar untuk koordinat game
struct Vector3 {
    float x, y, z;
};
struct Vector2 {
    float x, y;
};

static int enable_hack = 0;
static char *game_data_dir = nullptr;
static const char *game_package_name = "com.dts.freefiremax"; 

// Fungsi pembantu untuk membaca memori RAM (Memory Reading)
template <typename T>
T BacaMemori(uintptr_t address) {
    if (address == 0) return T();
    return *reinterpret_cast<T*>(address);
}

// Fungsi Utama untuk Menggambar ESP Line
void RenderESPLine(uintptr_t il2cppBase) {
    uintptr_t mainCameraFunc = BacaMemori<uintptr_t>(il2cppBase + OFFSET_GET_MAIN);
    if (!mainCameraFunc) return;

    int totalPemain = BacaMemori<int>(il2cppBase + OFFSET_GET_PLAYER_COUNT);
    if (totalPemain <= 0 || totalPemain > 100) return; 

    uintptr_t localPlayer = BacaMemori<uintptr_t>(il2cppBase + OFFSET_GET_LOCAL_PLAYER);

    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    float layarLebar = ImGui::GetIO().DisplaySize.x;
    float layarTinggi = ImGui::GetIO().DisplaySize.y;
    
    ImVec2 titikAwalGaris = ImVec2(layarLebar / 2.0f, layarTinggi);

    for (int i = 0; i < totalPemain; i++) {
        uintptr_t playerTarget = BacaMemori<uintptr_t>(il2cppBase + OFFSET_GET_PLAYER_BY_INDEX + (i * sizeof(void*)));
        if (!playerTarget) continue;

        if (playerTarget == localPlayer) continue;

        bool apakahMati = BacaMemori<bool>(playerTarget + OFFSET_IS_DEAD);
        if (apakahMati) continue;

        Vector3 posisi3DMusuh = BacaMemori<Vector3>(playerTarget + OFFSET_GET_POSITION);

        Vector2 posisi2DLayar;
        typedef bool (*t_WorldToScreen)(uintptr_t camera, Vector3 worldPos, Vector2& screenPos);
        t_WorldToScreen WorldToScreen = (t_WorldToScreen)(il2cppBase + OFFSET_WORLD_TO_SCREEN);

        if (WorldToScreen(mainCameraFunc, posisi3DMusuh, posisi2DLayar)) {
            ImVec2 titikMusuh = ImVec2(posisi2DLayar.x, posisi2DLayar.y);
            drawList->AddLine(titkAwalGaris, titikMusuh, IM_COL32(255, 0, 0, 255), 2.0f);
        }
    }
}

// Fungsi pencocokan paket game target 
bool isGame(JNIEnv *env, jstring appDataDir) {
    if (!appDataDir) return false;
    const char *app_data_dir = env->GetStringUTFChars(appDataDir, nullptr);
    if (!app_data_dir) return false;

    if (strstr(app_data_dir, game_package_name) != nullptr) {
        game_data_dir = new char[strlen(app_data_dir) + 1];
        strcpy(game_data_dir, app_data_dir);
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return true;
    }
    env->ReleaseStringUTFChars(appDataDir, app_data_dir);
    return false;
}

// Cara murni Linux Android untuk mencari Base Address (SUDAH DIPERBAIKI ARRAY-NYA)
uintptr_t AmbilBaseAddress(const char* libName) {
    uintptr_t startAddress = 0;
    char line[512]; // <--- Sudah diperbaiki menjadi array berukuran 512 karakter
    FILE* fp = fopen("/proc/self/maps", "rt");
    if (fp != nullptr) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, libName) != nullptr) {
                if (sscanf(line, "%lx-", &startAddress) == 1) {
                    break;
                }
            }
        }
        fclose(fp);
    }
    return startAddress;
}

void drawMenu() {
    uintptr_t il2cppBaseAddress = AmbilBaseAddress(targetLibName);
    if (il2cppBaseAddress != 0) {
        RenderESPLine(il2cppBaseAddress);
    }
    ImGui::ShowDemoWindow();
}

void *hack_thread(void *) {
    LOGI("Hack thread berhasil dijalankan.");
    
    uintptr_t il2cppBaseAddress = 0;
    do {
        sleep(1);
        il2cppBaseAddress = AmbilBaseAddress(targetLibName);
    } while (il2cppBaseAddress == 0);

    LOGI("Library game terdeteksi pada alamat memori!");
    return nullptr;
}
