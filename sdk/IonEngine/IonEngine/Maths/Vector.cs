using System.Runtime.InteropServices;

namespace IonEngine.Maths;

[StructLayout(LayoutKind.Sequential)]
public struct Vector2(float x, float y)
{
    public float x = x;
    public float y = y;

    public Vector2(float scalar) : this(scalar, scalar) { }
    
    public override string ToString()
    {
        return $"({x}, {y})";
    }
}

[StructLayout(LayoutKind.Sequential)]
public struct Vector3(float x, float y, float z)
{
    public float x = x;
    public float y = y;
    public float z = z;
    
    public Vector2 xy => new(x, y);
    public Vector2 yx => new(y, x);
    
    public Vector3 xzy => new(y, z, x);
    public Vector3 yxz => new(y, x, z);
    public Vector3 yzx => new(y, z, x);
    public Vector3 zxy => new(z, x, y);
    public Vector3 zyx => new(z, y, x);

    public Vector3(float scalar) : this(scalar, scalar, scalar) { }
    
    public float Length() => (float)Math.Sqrt(x * x + y * y + z * z);
    public static float Distance(Vector3 lhs, Vector3 rhs) => (lhs-rhs).Length();
    
    public static Vector3 operator +(Vector3 rhs, Vector3 lhs) => new(rhs.x + lhs.x, rhs.y + lhs.y, rhs.z + lhs.z);
    public static Vector3 operator -(Vector3 rhs, Vector3 lhs) => new(rhs.x - lhs.x, rhs.y - lhs.y, rhs.z - lhs.z);
    
    public static Vector3 operator *(Vector3 rhs, Vector3 lhs) => new(rhs.x * lhs.x, rhs.y * lhs.y, rhs.z * lhs.z);
    public static Vector3 operator /(Vector3 rhs, Vector3 lhs) => new(rhs.x / lhs.x, rhs.y / lhs.y, rhs.z / lhs.z);
    
    public static Vector3 operator *(Vector3 rhs, float scalar) => new(rhs.x * scalar, rhs.y * scalar, rhs.z * scalar);
    public static Vector3 operator /(Vector3 rhs, float scalar) => new(rhs.x / scalar, rhs.y / scalar, rhs.z / scalar);
    
    public override string ToString()
    {
        return $"({x}, {y}, {z})";
    }
}

[StructLayout(LayoutKind.Sequential)]
public struct Vector4(float x, float y, float z, float w)
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