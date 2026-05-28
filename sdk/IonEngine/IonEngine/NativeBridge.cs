using Coral.Managed.Interop;
using IonEngine.Maths;
// ReSharper disable InconsistentNaming

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
    
    // MeshRenderer
    internal static delegate*<MeshRenderer*, uint> MeshRenderer_getMesh;
    internal static delegate*<MeshRenderer*, uint, void> MeshRenderer_setMesh;
    
    internal static delegate*<MeshRenderer*, uint> MeshRenderer_getMaterial;
    internal static delegate*<MeshRenderer*, uint, void> MeshRenderer_setMaterial;
    
    // Mesh
    internal static delegate*<IntPtr, NativeArray<Vertex>> Mesh_getVertices;
    internal static delegate*<IntPtr, NativeArray<Vertex>, void> Mesh_setVertices;
    
    internal static delegate*<IntPtr, NativeArray<int>> Mesh_getIndices;
    internal static delegate*<IntPtr, NativeArray<int>, void> Mesh_setIndices;
    
    internal static delegate*<uint> Mesh_create; // returns the handle
    internal static delegate*<uint, IntPtr> Mesh_get; // returns the ptr
    internal static delegate*<uint, void> Mesh_release;
    
    // Material Instance
    internal static delegate*<uint> MaterialInstance_create;
    internal static delegate*<uint, IntPtr> MaterialInstance_get;
    internal static delegate*<uint, void> MaterialInstance_release;
#pragma warning restore CS0649
}           