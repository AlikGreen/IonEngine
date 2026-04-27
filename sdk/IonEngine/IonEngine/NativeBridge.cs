using Coral.Managed.Interop;
using IonEngine.Maths;

namespace IonEngine;

internal static unsafe class NativeBridge
{
#pragma warning disable CS0649
    // Input
    internal static delegate*<int, bool> Input_isKeyHeld;
    internal static delegate*<int, bool> Input_isKeyPressed;
    internal static delegate*<int, bool> Input_isKeyReleased;
    
    //Registry
    internal static delegate*<NativeString, ulong>  Registry_getTypeHash;
    internal static delegate*<IntPtr, ulong, ulong, ulong, void>  Registry_registerType;
    
    internal static delegate*<IntPtr, ulong, ulong, IntPtr>  Registry_getEntityComponent;
    internal static delegate*<IntPtr, NativeArray<ulong>, IntPtr> Registry_createView;
    
    internal static delegate*<ulong, ulong>       Registry_createEntity;
    internal static delegate*<ulong, ulong, void> Registry_destroyEntity;
    
    // View
    internal static delegate*<IntPtr, ulong, EntityComponentData> View_getAtIndex;
    internal static delegate*<IntPtr, ulong> View_getSize;
    
    // Log
    internal static delegate*<NativeString, void> Log_logInfo;
    
    // Engine
    internal static delegate*<IntPtr> Engine_getCurrentRegistry;
    internal static delegate*<float> Engine_getDeltaTime;
    
    // Tag
    internal static delegate*<Tag*, NativeString>       Tag_getName;
    internal static delegate*<Tag*, NativeString, void> Tag_setName;
    
    // Transform
    internal static delegate*<Transform*, Vector3> Transform_getForward;
    internal static delegate*<Transform*, Vector3> Transform_getRight;
    internal static delegate*<Transform*, Vector3> Transform_getUp;
#pragma warning restore CS0649
}           