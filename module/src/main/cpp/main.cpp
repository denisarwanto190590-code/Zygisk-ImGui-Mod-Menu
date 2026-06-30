#include "imgui.h"
#include <cmath>

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

// Fungsi pembantu untuk membaca memori RAM (Memory Reading)
template <typename T>
T BacaMemori(uintptr_t address) {
    if (address == 0) return T();
    return *reinterpret_cast<T*>(address);
}

// Fungsi Utama untuk Menggambar ESP Line Free Fire Max
void RenderESPLine(uintptr_t il2cppBase) {
    // 1. Ambil Kamera Utama Game untuk hitung posisi layar
    uintptr_t mainCameraFunc = BacaMemori<uintptr_t>(il2cppBase + OFFSET_GET_MAIN);
    if (!mainCameraFunc) return;

    // 2. Ambil data total pemain di dalam room/match
    int totalPemain = BacaMemori<int>(il2cppBase + OFFSET_GET_PLAYER_COUNT);
    if (totalPemain <= 0 || totalPemain > 100) return; // Batasi perulangan agar tidak crash

    // 3. Ambil data akun kita sendiri (Local Player)
    uintptr_t localPlayer = BacaMemori<uintptr_t>(il2cppBase + OFFSET_GET_LOCAL_PLAYER);

    // Dapatkan data layar dari ImGui
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    float layarLebar = ImGui::GetIO().DisplaySize.x;
    float layarTinggi = ImGui::GetIO().DisplaySize.y;
    
    // Titik awal garis: Tengah bawah layar HP Anda
    ImVec2 titikAwalGaris = ImVec2(layarLebar / 2.0f, layarTinggi);

    // 4. Lakukan Perulangan (Looping) untuk memeriksa semua musuh
    for (int i = 0; i < totalPemain; i++) {
        // Panggil pemain berdasarkan urutan index (0, 1, 2, dst)
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

        // Jika posisi musuh berhasil diubah ke format layar HP (ada di depan kamera)
        if (WorldToScreen(mainCameraFunc, posisi3DMusuh, posisi2DLayar)) {
            ImVec2 titikMusuh = ImVec2(posisi2DLayar.x, posisi2DLayar.y);

            // 7. Gambar Garis ESP warna Merah Terang
            drawList->AddLine(titikAwalGaris, titikMusuh, IM_COL32(255, 0, 0, 255), 2.0f);
        }
    }
}
