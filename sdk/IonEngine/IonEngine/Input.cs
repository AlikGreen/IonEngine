using System.Runtime.InteropServices;

namespace IonEngine;

public class Input
{ 
    public static bool IsKeyHeld(KeyCode keyCode)
    {
        unsafe { return NativeBridge.Input_isKeyHeld((int)keyCode); }
    }

    public static bool IsKeyPressed(KeyCode keyCode)
    {
        unsafe { return NativeBridge.Input_isKeyPressed((int)keyCode); }
    }
    
    public static bool IsKeyReleased(KeyCode keyCode)
    {
        unsafe { return NativeBridge.Input_isKeyReleased((int)keyCode); }
    }
    
}
