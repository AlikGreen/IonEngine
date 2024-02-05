using System.Numerics;
using Jitter.LinearMath;

namespace IonEngine;

public static class Maths
{
    public static float DegreesToRadians(float degrees)
    {
        return degrees * ((float)Math.PI / 180f);
    }

    public static Vector3 RadiansToDegrees(Vector3 degrees)
    {
        return degrees * (180f / (float)Math.PI);
    }

    public static Vector3 MatrixToEuler(JMatrix matrix)
    {
        Vector3 euler;
        float sy = (float)Math.Sqrt(matrix.M11 * matrix.M11 +  matrix.M21 * matrix.M21);

        bool singular = sy < 1e-6;  // If

        if (!singular)
        {
            euler.X = (float)Math.Atan2(matrix.M32, matrix.M33);
            euler.Y = (float)Math.Atan2(-matrix.M31, sy);
            euler.Z = (float)Math.Atan2(matrix.M21, matrix.M11);
        }
        else
        {
            euler.X = (float)Math.Atan2(-matrix.M23, matrix.M22);
            euler.Y = (float)Math.Atan2(-matrix.M31, sy);
            euler.Z = 0;
        }
        return euler;
    }
}