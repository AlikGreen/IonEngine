using System.Runtime.InteropServices;

namespace NeonEngine;

public class Input
{
#pragma warning disable CS0649
    internal static unsafe delegate*<int, bool> isKeyHeldCall;
    internal static unsafe delegate*<int, bool> isKeyPressedCall;
    internal static unsafe delegate*<int, bool> isKeyReleasedCall;
#pragma warning restore CS0649

    public static bool IsKeyHeld(KeyCode keyCode)
    {
        unsafe { return isKeyHeldCall((int)keyCode); }
    }

    public static bool IsKeyPressed(KeyCode keyCode)
    {
        unsafe { return isKeyPressedCall((int)keyCode); }
    }
    
    public static bool IsKeyReleased(KeyCode keyCode)
    {
        unsafe { return isKeyReleasedCall((int)keyCode); }
    }

    public static void Run()
    {
        Scene scene = SceneManager.GetCurrentScene();
        var view  = scene.View<Tag>();
        
        Log.Info($"{view.Length}");
        
        view.ForEach((ulong entity, ref Tag tag) =>
        {
            Log.Info($"[{entity}] pos: {tag.name}");
        });
        
        Log.Info("looped");
    }
}
