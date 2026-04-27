namespace IonEngine;

public static class SceneManager
{
    public static Scene GetCurrentScene()
    {
        unsafe
        {
            return new Scene(NativeBridge.Engine_getCurrentRegistry());   
        }
    }
}