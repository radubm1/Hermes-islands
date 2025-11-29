cmake_minimum_required(VERSION 3.16)
project(hermes_islands CXX)
set(CMAKE_CXX_STANDARD 17)

# Setează calea către JDK-ul complet instalat
set(JAVA_HOME "C:/Program Files/Java/jdk-25")  # actualizează dacă ai alt JDK

include_directories(
    ${JAVA_HOME}/include
    ${JAVA_HOME}/include/win32
    ${CMAKE_SOURCE_DIR}/src
)

# Librăria de bază hermescore
add_library(hermescore STATIC
    src/event.hpp
    src/event_bus.hpp
    src/event_bus.cpp
    src/island.hpp
    src/island.cpp   # <- asigură-te că island.cpp e inclus
    src/vm.hpp
    src/vm.cpp
)

# Executabilul principal
add_executable(hermes_islands
    src/main.cpp
)

# Link executabilul cu hermescore și jvm.lib
target_link_libraries(hermes_islands PRIVATE hermescore "${JAVA_HOME}/lib/jvm.lib")

# DLL-ul hermesbridge pentru JNI
add_library(hermesbridge SHARED
    src/hermesbridge.cpp
    src/event.hpp
    src/vm.hpp
)

target_link_libraries(hermesbridge PRIVATE hermescore "${JAVA_HOME}/lib/jvm.lib")