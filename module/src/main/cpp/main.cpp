// --- MAIN HACK THREAD ---
void *hack_thread(void *) {
    unsigned long il2cpp_base = 0;
    
    // Tunggu sampai library Free Fire termuat di memori
    while (!il2cpp_base) {
        il2cpp_base = get_module_base("libil2cpp.so");
        sleep(1);
    }

    // [HOOK 1] Mengunci offset get_Position yang Anda temukan sebelumnya
    DobbyHook((void *)(il2cpp_base + 0x7CEE8A0), (void *)hk_GetPosition, (void **)&old_GetPosition);

    // [HOOK 2] Mengunci offset Kamera UGCAPIWorldToScreenPoint yang baru saja ketemu
    DobbyHook((void *)(il2cpp_base + 0x80BFB2C), (void *)hk_WorldToScreenPoint, (void **)&old_WorldToScreenPoint);

    return nullptr;
}
