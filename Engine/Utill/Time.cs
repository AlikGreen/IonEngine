namespace IonEngine;

public static class Time
{
    public static float deltaTime = 0;

    public static void OnUpdate(float dt)
    {
        deltaTime = dt;
    }
}