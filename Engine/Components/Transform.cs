using System.Numerics;

namespace IonEngine;

public class Transform : Component
{
    public Vector3 position = new Vector3();
    public Vector3 scale = new Vector3(1.0f);
    public Vector3 rotation = new Vector3();
}