//
// Created by Reveny on 2023/1/13.
// Updated for Free Fire Max ESP Line
//
#define targetLibName OBFUSCATE("libil2cpp.so")

#include <jni.h>
#include <sys/system_properties.h>
#include <cmath>

#include "KittyMemory/MemoryPatch.h"
#include "Includes/ESP.h"
#include "Includes/Dobby/dobby.h"
#include "Includes/Utils.h"
#include "Includes/ImGui.h"

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

static int enable_hack;
static char *game_data_dir = nullptr;
static char *game_package_name = "com.dts.freefiremax"; // Sudah diubah ke Free Fire Max

// Fungsi pembantu untuk membaca memori RAM (Memory Reading)
template <typename T>
T BacaMemori(uintptr_t address) {
    if (address == 0) return T();
    return *reinterpret_cast<T*>(address);
}

// Fungsi Utama untuk Menggambar ESP Line
void RenderESPLine(uintptr_t il2cppBase) {
    // 1. Ambil Kamera Utama Game untuk hitung posisi layar
    uintptr_t mainCameraFunc = BacaMemori<uintptr_t>(il2cppBase + OFFSET_GET_MAIN);
    if (!mainCameraFunc) return;

    // 2. Ambil data total pemain di dalam room/match
    int totalPemain = BacaMemori<int>(il2cppBase + OFFSET_GET_PLAYER_COUNT);
    if (totalPemain <= 0 || totalPemain > 100) return; // Batasi agar tidak crash

    // 3. Ambil data akun kita sendiri (Local Player)
    uintptr_t localPlayer = BacaMemori<uintptr_t>(il2cppBase + OFFSET_GET_LOCAL_PLAYER);

    // Dapatkan data dimensi layar dari ImGui
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    float layarLebar = ImGui::GetIO().DisplaySize.x;
    float layarTinggi = ImGui::GetIO().DisplaySize.y;
    
    // Titik awal garis: Tengah bawah layar HP Anda
    ImVec2 titikAwalGaris = ImVec2(layarLebar / 2.0f, layarTinggi);

    // 4. Perulangan (Looping) untuk memeriksa semua musuh
    for (int i = 0; i < totalPemain; i++) {
        // Panggil pemain berdasarkan urutan index
        uintptr_t playerTarget = BacaMemori<uintptr_t>(il2cppBase + OFFSET_GET_PLAYER_BY_INDEX + (i * sizeof(void*)));
        if (!playerTarget) continue;

        // Validasi: Jangan menggambar garis ke diri kita sendiri
        if (playerTarget == localPlayer) continue;

        // Validasi: Cek apakah musuh sudah mati atau belum
        bool apakahMati = BacaMemori<bool>(playerTarget + OFFSET_IS_DEAD);
        if (apakahMati) continue;

        // 5. Ambil Koordinat 3D posisi musuh di peta game
        Vector3 posisi3DMusuh = BacaMemori<Vector3>(playerTarget + OFFSET_GET_POSITION);

        // 6. Jalankan rumus World-To-Screen (W2S) menggunakan fungsi bawaan game
        Vector2 posisi2DLayar;
        typedef bool (*t_WorldToScreen)(uintptr_t camera, Vector3 worldPos, Vector2& screenPos);
        t_WorldToScreen WorldToScreen = (t_WorldToScreen)(il2cppBase + OFFSET_WORLD_TO_SCREEN);

        // Jika posisi musuh berhasil diubah ke format layar HP
        if (WorldToScreen(mainCameraFunc, posisi3DMusuh, posisi2DLayar)) {
            ImVec2 titikMusuh = ImVec2(posisi2DLayar.x, posisi2DLayar.y);

            // 7. Gambar Garis ESP warna Merah Terang (ketebalan 2 piksel)
            drawList->AddLine(titkAwalGaris, titikMusuh, IM_COL32(255, 0, 0, 255), 2.0f);
        }
    }
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

void drawMenu() {
    // Jalankan kalkulasi dan gambar garis ESP Line Free Fire Max
    uintptr_t il2cppBaseAddress = getBaseAddress(targetLibName);
    if (il2cppBaseAddress != 0) {
        RenderESPLine(il2cppBaseAddress);
    }

    // Menampilkan menu bawaan ImGui
    ImGui::ShowDemoWindow();
}

void *hack_thread(void *) {
    LOGI(OBFUSCATE("hack thread: %d"), gettid());
    initModMenu((void *)drawMenu);

    //Check if target lib is loaded
    do {
        LOGI(OBFUSCATE("Trying to find lib... PID: %d"), getpid());
        sleep(1);
    } while (!isLibraryLoaded(targetLibName));

    LOGI(OBFUSCATE("%s has been loaded, 0x%lx"), (const char *) targetLibName, getBaseAddress(targetLibName));

    //Hooks and Patches here

    LOGI(OBFUSCATE("Done"));
    return nullptr;
}
