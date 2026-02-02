using System.Runtime.InteropServices;
using Coral.Managed.Interop;

namespace NeonEngine;

[AttributeUsage(AttributeTargets.Struct)]
public class NativeComponentAttribute : Attribute
{
    public NativeComponentAttribute()
    {
    }
}

[NativeComponent]
[StructLayout(LayoutKind.Sequential)]
struct Tag
{
#pragma warning disable CS0649
    internal static unsafe delegate*<Tag*, NativeString> getNameCall;
    internal static unsafe delegate*<Tag*, NativeString, void> setNameCall;
#pragma warning restore CS0649
    
    private unsafe fixed byte data[56];

    public string name
    {
        get
        {
            unsafe
            {
                fixed (Tag* ptr = &this)
                {
                    return getNameCall(ptr).ToString()!;
                }
            }
        }
        set
        {
            unsafe
            {
                fixed (Tag* ptr = &this)
                {
                    setNameCall(ptr, value);
                }
            }
        }
    }
}

[NativeComponent]
[StructLayout(LayoutKind.Sequential)]
struct Camera
{
    private unsafe fixed byte data[304];
}

[StructLayout(LayoutKind.Sequential)]
struct Vector3(float x, float y, float z)
{
    public float x = x;
    public float y = y;
    public float z = z;

    public Vector3(float scalar) : this(scalar, scalar, scalar) { }
    
    public override string ToString()
    {
        return $"({x}, {y}, {z})";
    }
}

[StructLayout(LayoutKind.Sequential)]
struct Vector4(float x, float y, float z, float w)
{
    public float x = x;
    public float y = y;
    public float z = z;
    public float w = w;

    public Vector4(float scalar) : this(scalar, scalar, scalar, scalar) { }
    
    public override string ToString()
    {
        return $"({x}, {y}, {z}, {w})";
    }
}

[StructLayout(LayoutKind.Sequential)]
struct Matrix4
{
    public Vector4 row1;
    public Vector4 row2;
    public Vector4 row3;
    public Vector4 row4;
}


[NativeComponent]
[StructLayout(LayoutKind.Sequential)]
struct Transform
{
    // Must match C++ layout exactly
    private Vector3 _position = new(0.0f);
    private Vector3 _rotation = new(0.0f);
    private Vector3 _scale = new(1.0f);

    private bool dirty = true;
    private Matrix4 _cachedLocalMatrix;
    private Matrix4 _cachedLocalRotationMatrix;

    public Transform()
    {
        
    }

    public Vector3 position
    {
        get => _position;
        set
        {
            dirty = true;
            _position = value;
        }
    }

    public Vector3 rotation
    {
        get => _rotation;
        set
        {
            dirty = true;
            _rotation = value;
        }
    }

    public Vector3 scale
    {
        get => _scale;
        set
        {
            dirty = true;
            _scale = value;
        }
    }
}
