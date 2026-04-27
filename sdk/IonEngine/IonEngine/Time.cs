namespace IonEngine;

public static class Time
{
    public static float deltaTime
    {
        get
        {
            unsafe
            {
                return NativeBridge.Engine_getDeltaTime();
            }
        }
    }
}