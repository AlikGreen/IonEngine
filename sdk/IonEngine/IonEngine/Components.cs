using System.Runtime.InteropServices;
using Coral.Managed.Interop;
using IonEngine.Maths;

namespace IonEngine;

[AttributeUsage(AttributeTargets.Struct)]
public class NativeComponentAttribute : Attribute
{
    public NativeComponentAttribute()
    {
    }
}

[NativeComponent]
[StructLayout(LayoutKind.Sequential)]
public struct Tag
{
    
    private unsafe fixed byte data[56];

    public string name
    {
        get
        {
            unsafe
            {
                fixed (Tag* ptr = &this)
                {
                    return NativeBridge.Tag_getName(ptr).ToString()!;
                }
            }
        }
        set
        {
            unsafe
            {
                fixed (Tag* ptr = &this)
                {
                    NativeBridge.Tag_setName(ptr, value);
                }
            }
        }
    }
}

[NativeComponent]
[StructLayout(LayoutKind.Sequential)]
public struct Camera
{
    
}

[NativeComponent]
[StructLayout(LayoutKind.Sequential)]
public struct Transform
{
    public Vector3 position = new(0.0f);
    public Vector3 rotation = new(0.0f);
    public Vector3 scale = new(1.0f);

    public Vector3 forward
    {
        get
        {
            unsafe
            {
                fixed (Transform* ptr = &this)
                {
                    return NativeBridge.Transform_getForward(ptr);
                }
            }
        }
    }
    
    public Vector3 right
    {
        get
        {
            unsafe
            {
                fixed (Transform* ptr = &this)
                {
                    return NativeBridge.Transform_getRight(ptr);
                }
            }
        }
    }
    
    public Vector3 up
    {
        get
        {
            unsafe
            {
                fixed (Transform* ptr = &this)
                {
                    return NativeBridge.Transform_getUp(ptr);
                }
            }
        }
    }

    public Transform()
    {
    }
}

[NativeComponent]
[StructLayout(LayoutKind.Sequential)]
public struct MeshRenderer
{
    public Mesh? mesh
    {
        get
        {
            unsafe
            {
                fixed (MeshRenderer* ptr = &this)
                {
                    uint id = NativeBridge.MeshRenderer_getMesh(ptr);
                    return id == 0 ? null : new Mesh(id);
                }
            }
        }
        set
        {
            unsafe
            {
                fixed (MeshRenderer* ptr = &this)
                {
                    NativeBridge.MeshRenderer_setMesh(ptr, value?.id ?? 0);
                }
            }
        }
    }
    
    public Material? material
    {
        get
        {
            unsafe
            {
                fixed (MeshRenderer* ptr = &this)
                {
                    uint id = NativeBridge.MeshRenderer_getMaterial(ptr);
                    return id == 0 ? null : new Material(id);
                }
            }
        }
        set
        {
            unsafe
            {
                fixed (MeshRenderer* ptr = &this)
                {
                    NativeBridge.MeshRenderer_setMaterial(ptr, value?.id ?? 0);
                }
            }
        }
    }
}
