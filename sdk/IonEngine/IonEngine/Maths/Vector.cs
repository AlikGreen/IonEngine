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
    public static Vector3 up = new(0, 1, 0);
    public static Vector3 down = new(0, -1, 0);
    public static Vector3 left = new(-1, 0, 0);
    public static Vector3 right = new(1, 0, 0);
    public static Vector3 forward = new(0, 0, 1);
    public static Vector3 back = new(0, 0, -1);
    
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
    
    public Vector3 Normalized() => this / Length();
    public float Dot(Vector3 other) => x * other.x + y * other.y + z * other.z;
    public float Length() => (float)Math.Sqrt(x * x + y * y + z * z);
    public static float Distance(Vector3 lhs, Vector3 rhs) => (lhs-rhs).Length();
    public static float Dot(Vector3 lhs, Vector3 rhs) => lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
    public static Vector3 Cross(Vector3 lhs, Vector3 rhs) => new(lhs.y*rhs.z - lhs.z*rhs.y, lhs.z*rhs.x - lhs.x*rhs.z, lhs.x*rhs.y - lhs.y*rhs.x);
    
    public static Vector3 operator +(Vector3 lhs, Vector3 rhs) => new(rhs.x + lhs.x, rhs.y + lhs.y, rhs.z + lhs.z);
    public static Vector3 operator -(Vector3 lhs, Vector3 rhs) => new(rhs.x - lhs.x, rhs.y - lhs.y, rhs.z - lhs.z);

    public static Vector3 operator *(Vector3 lhs, Vector3 rhs) => new(rhs.x * lhs.x, rhs.y * lhs.y, rhs.z * lhs.z);
    public static Vector3 operator /(Vector3 lhs, Vector3 rhs) => new(rhs.x / lhs.x, rhs.y / lhs.y, rhs.z / lhs.z);
    
    public static Vector3 operator *(Vector3 lhs, float scalar) => new(lhs.x * scalar, lhs.y * scalar, lhs.z * scalar);
    public static Vector3 operator *(float scalar, Vector3 rhs) => new(rhs.x * scalar, rhs.y * scalar, rhs.z * scalar);
    public static Vector3 operator /(Vector3 lhs, float scalar) => new(lhs.x / scalar, lhs.y / scalar, lhs.z / scalar);

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