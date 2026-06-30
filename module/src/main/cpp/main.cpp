//
// Created by Reveny on 2023/1/13.
//

#include <zenith.h>
#include "modmenu.h"

REGISTER_ZYGISK_MODULE(ZenithModule)
REGISTER_ZYGISK_COMPANION(ZenithCompanion)

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
