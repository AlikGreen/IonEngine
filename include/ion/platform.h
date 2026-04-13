#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define ION_PLATFORM_WINDOWS
    #if defined(_WIN64)
        #define ION_PLATFORM_WINDOWS_64
    #else
        #define ION_PLATFORM_WINDOWS_32
    #endif

#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC
        #define ION_PLATFORM_MAC
    #elif TARGET_OS_IPHONE
        #define ION_PLATFORM_IOS
    #endif

#elif defined(__linux__)
    #define ION_PLATFORM_LINUX

#elif defined(__ANDROID__)
    #define ION_PLATFORM_ANDROID

#else
    #error "Unknown platform"
#endif

#if defined(_MSC_VER)
    #define ION_COMPILER_MSVC
    #define ION_COMPILER_VERSION _MSC_VER

#elif defined(__clang__)
    #define ION_COMPILER_CLANG
    #define ION_COMPILER_VERSION (__clang_major__ * 100 + __clang_minor__)

#elif defined(__GNUC__)
    #define ION_COMPILER_GCC
    #define ION_COMPILER_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)

#else
    #error "Unknown compiler"
#endif

#if defined(_M_X64) || defined(__x86_64__)
    #define ION_ARCH_X64
#elif defined(_M_IX86) || defined(__i386__)
    #define ION_ARCH_X86
#elif defined(_M_ARM64) || defined(__aarch64__)
    #define ION_ARCH_ARM64
#elif defined(_M_ARM) || defined(__arm__)
    #define ION_ARCH_ARM
#endif

#if defined(NDEBUG)
    #define ION_RELEASE
#else
    #define ION_DEBUG
#endif

#ifdef ION_PLATFORM_WINDOWS
    #define ION_BREAK() __debugbreak()
#elif defined(ION_COMPILER_CLANG) || defined(ION_COMPILER_GCC)
    #define ION_BREAK() __builtin_trap()
#else
    #define ION_BREAK() (void)0
#endif

namespace ion::Platform
{
    std::filesystem::path getExecutablePath();

    void launch(const std::filesystem::path& executablePath,
                const std::string& args = "");
}