#pragma once
#include "enumFlags.h"

namespace ion
{
enum class SystemTags : uint64_t
{
    None            = 0,

    // Core simulation
    Simulation      = 1ull << 0,
    Physics         = 1ull << 1,
    Scripting       = 1ull << 2,
    Animation       = 1ull << 3,
    AI              = 1ull << 4,
    Particles       = 1ull << 5,

    // Rendering
    Rendering       = 1ull << 6,
    UI              = 1ull << 7,

    // Audio
    Audio           = 1ull << 8,

    // Input
    Input           = 1ull << 9,

    // Networking
    Network         = 1ull << 10,

    // Assets
    Assets  = 1ull << 11,

    // Gameplay states
    Gameplay        = 1ull << 12,
    Cutscene        = 1ull << 13,
    Menu            = 1ull << 14,
    Loading         = 1ull << 15,

    // Debug / dev
    Debug           = 1ull << 16,
    Profiling       = 1ull << 17,

    // Common composites
    AllSimulation   = Simulation | Physics | Scripting | Animation | AI | Particles,
    AllRendering    = Rendering | UI,
    AllDebug        = Debug | Profiling,
};

ION_DEFINE_ENUM_FLAGS(SystemTags);
}
