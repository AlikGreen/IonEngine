namespace NeonEngine;

public static class SceneManager
{
#pragma warning disable CS0649
    internal static unsafe delegate*<IntPtr> getCurrentRegistryCall;
#pragma warning restore CS0649
    public static Scene GetCurrentScene()
    {
        unsafe
        {
            return new Scene(getCurrentRegistryCall());   
        }
    }
}