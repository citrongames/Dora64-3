#include "android_platform.hpp"
#ifdef __ANDROID__
#include <SDL.h>
#include <jni.h>
#include <cstdio>
#include <thread>
#include <chrono>
namespace dora3::android {
Paths prepare(){
    const char* internal=SDL_AndroidGetInternalStoragePath();
    const char* external=SDL_AndroidGetExternalStoragePath();
    if(!internal||!external||(SDL_AndroidGetExternalStorageState()&SDL_ANDROID_EXTERNAL_STORAGE_WRITE)==0)return {};
    Paths p{std::filesystem::u8path(internal),std::filesystem::u8path(external),{}};
    std::error_code ec;std::filesystem::create_directories(p.user,ec);if(ec)return {};
    const auto candidate=p.data/"dora3.n64.jp.z64";
    while(true){
        if(std::filesystem::exists(p.data/".rom-selection-canceled",ec))return {};
        if(std::filesystem::exists(p.data/".assets-ready",ec)&&std::filesystem::is_regular_file(candidate,ec))break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    p.rom=candidate;
    // Separate app ID/storage: never migrates or opens Dora64's settings/saves.
#ifdef DORA3_DIAGNOSTICS
    std::freopen((p.user/"native-stderr.log").c_str(),"w",stderr);
    std::setvbuf(stderr,nullptr,_IONBF,0);
#endif
    return p;
}
void shader_loading(bool visible){
    auto* env=static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    auto activity=static_cast<jobject>(SDL_AndroidGetActivity());
    if(!env||!activity)return;
    auto cls=env->GetObjectClass(activity);
    auto method=env->GetMethodID(cls,"setShaderCompilationVisible","(Z)V");
    if(method)env->CallVoidMethod(activity,method,jboolean(visible));
    if(env->ExceptionCheck())env->ExceptionClear();
    env->DeleteLocalRef(cls);env->DeleteLocalRef(activity);
}
}
#else
namespace dora3::android {Paths prepare(){return {};} void shader_loading(bool){} }
#endif
