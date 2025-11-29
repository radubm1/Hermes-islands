#include <jni.h>
#include <cstdio>

// Dacă ai un EventBus C++ la care vrei să trimiți rezultatele,
// îl expui aici printr-o interfață C sau un singleton.
// Pentru demo, doar logăm în consolea JVM prin printf.

extern "C" {

// emitResult(double): (D)V
JNIEXPORT void JNICALL
Java_engine_SimulationContext_emitResult(JNIEnv* env, jobject thiz, jdouble value) {
    // Nu folosi GetStringUTFChars aici — parametrul este un jdouble
    // Log simplu; poți emite în EventBus-ul tău C++ dacă ai un pointer global/singleton
    std::printf("[HermesBridge] emitResult(double): %f\n", static_cast<double>(value));
    std::fflush(stdout);
}

// (Opțional) emitResultText(String): (Ljava/lang/String;)V
JNIEXPORT void JNICALL
Java_engine_SimulationContext_emitResultText(JNIEnv* env, jobject thiz, jstring msg) {
    if (msg == nullptr) {
        std::printf("[HermesBridge] emitResultText(null)\n");
        std::fflush(stdout);
        return;
    }

    const char* utf = env->GetStringUTFChars(msg, nullptr);
    if (!utf) {
        // OOM sau altă eroare
        std::printf("[HermesBridge] emitResultText(GetStringUTFChars failed)\n");
        std::fflush(stdout);
        return;
    }

    std::printf("[HermesBridge] emitResult(text): %s\n", utf);
    std::fflush(stdout);
    env->ReleaseStringUTFChars(msg, utf);
}

} // extern "C"