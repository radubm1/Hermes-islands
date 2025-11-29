#include "island.hpp"
#include "event.hpp"
#include <windows.h>
#include <filesystem>
#include <iostream>
#include <jni.h>

Island::Island(std::string name, std::size_t heapBudgetBytes, EventBus& bus)
    : name_(std::move(name)), budget_(heapBudgetBytes), bus_(bus) {}

bool Island::loadModule(const std::string& jarPath) {
    bus_.emit(Event{EventType::LOAD_REQUEST, jarPath});
    loaded_ = true;
    bus_.emit(Event{EventType::LOAD_OK, jarPath});
    return true;
}

bool Island::linkAll() {
    if (!loaded_) return false;
    bus_.emit(Event{EventType::VERIFY_OK, name_});
    bus_.emit(Event{EventType::LINK_OK, name_});
    linked_ = true;
    return true;
}

bool Island::unload() {
    bus_.emit(Event{EventType::UNLOAD_REQUEST, name_});
    bus_.emit(Event{EventType::UNLOAD_OK, name_});
    loaded_ = false;
    linked_ = false;
    return true;
}

bool Island::runMain(const std::string& mainClass) {
    if (!linked_) return false;
    bus_.emit(Event{EventType::INVOKE_START, mainClass});

    // 1. Setează classpath absolut către JAR-ul din plugins
    std::string cpOpt =
        "-Djava.class.path=C:/Users/radub/Documents/Hermes-islands-master/Hermes-islands-master/plugins/meanrev-1.2.0.jar";

    // 2. Setează java.home (JDK complet instalat)
    std::string javaHomeOpt = "-Djava.home=C:/Program Files/Java/jdk-25";

    // 3. Activează accesul nativ pentru JNI
    std::string nativeAccessOpt = "--enable-native-access=ALL-UNNAMED";

    // 4. Configurează JVM
    JavaVMOption options[3];
    options[0].optionString = const_cast<char*>(javaHomeOpt.c_str());
    options[1].optionString = const_cast<char*>(cpOpt.c_str());
    options[2].optionString = const_cast<char*>(nativeAccessOpt.c_str());

    JavaVMInitArgs vm_args{};
    vm_args.version = JNI_VERSION_1_8;
    vm_args.nOptions = 3;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = JNI_FALSE;

    JavaVM* jvm = nullptr;
    JNIEnv* env = nullptr;

    if (JNI_CreateJavaVM(&jvm, reinterpret_cast<void**>(&env), &vm_args) < 0 || !env) {
        std::cerr << "[Hermes] Failed to create JVM\n";
        bus_.emit(Event{EventType::UNLOAD_FAIL, name_});
        return false;
    }

    // 5. Găsește clasa și metoda main
    jclass cls = env->FindClass("com/example/meanrev/Main");
    if (!cls) {
        std::cerr << "[Hermes] Could not find class " << mainClass << "\n";
        jvm->DestroyJavaVM();
        return false;
    }

    jmethodID mid = env->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V");
    if (!mid) {
        std::cerr << "[Hermes] Could not find main method\n";
        jvm->DestroyJavaVM();
        return false;
    }

    jobjectArray args = env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);

    // 6. Rulează metoda main
    env->CallStaticVoidMethod(cls, mid, args);

    bus_.emit(Event{EventType::INVOKE_END, mainClass});
    jvm->DestroyJavaVM();
    return true;
}