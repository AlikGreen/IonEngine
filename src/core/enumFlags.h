#pragma once


#define ION_DEFINE_ENUM_FLAGS(T) \
inline T operator|(T a, T b)  { return static_cast<T>(static_cast<std::underlying_type_t<T>>(a) | static_cast<std::underlying_type_t<T>>(b)); } \
inline T operator&(T a, T b)  { return static_cast<T>(static_cast<std::underlying_type_t<T>>(a) & static_cast<std::underlying_type_t<T>>(b)); } \
inline T operator^(T a, T b)  { return static_cast<T>(static_cast<std::underlying_type_t<T>>(a) ^ static_cast<std::underlying_type_t<T>>(b)); } \
inline T operator~(T a)       { return static_cast<T>(~static_cast<std::underlying_type_t<T>>(a)); } \
inline T& operator|=(T& a, T b) { return a = a | b; } \
inline T& operator&=(T& a, T b) { return a = a & b; } \
inline T& operator^=(T& a, T b) { return a = a ^ b; } \
inline bool hasFlag(T val, T flag) { return (val & flag) != static_cast<T>(0); }