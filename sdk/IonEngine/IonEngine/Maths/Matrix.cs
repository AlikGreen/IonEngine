using System.Runtime.InteropServices;

namespace IonEngine.Maths;

[StructLayout(LayoutKind.Sequential)]
public struct Matrix4
{
    public Vector4 row1;
    public Vector4 row2;
    public Vector4 row3;
    public Vector4 row4;
}
