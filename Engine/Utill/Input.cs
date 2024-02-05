using System.Numerics;
using Silk.NET.Input;

namespace IonEngine;

public static class Input
{
    private static IKeyboard keyboard;
    private static IMouse mouse;

    public static Vector2 mousePosition;
    public static Vector2 mouseDelta;
    private static Vector2 lastMousePosition;

    private static List<Key> keys = new List<Key>();
    private static List<Key> keysDown = new List<Key>();
    private static List<Key> keysUp = new List<Key>();

    private static List<MouseButton> mouseButtons = new List<MouseButton>();
    private static List<MouseButton> mouseButtonsDown = new List<MouseButton>();
    private static List<MouseButton> mouseButtonsUp = new List<MouseButton>();

    public static bool enabled = true;

    public static void Init(IInputContext inputContext)
    {
        keyboard = inputContext.Keyboards.FirstOrDefault();
        mouse = inputContext.Mice.FirstOrDefault();
        keyboard.KeyDown += KeyDown;
        keyboard.KeyUp += KeyUp;
        mouse.MouseDown += MouseDown;
        mouse.MouseUp += MouseUp;
    }

    private static void KeyDown(IKeyboard keyboard, Key key, int i)
    {
        if(!enabled) return;

        if(!keys.Contains(key))
        {
            keysDown.Add(key);
            keys.Add(key);
        }
    }

    private static void KeyUp(IKeyboard keyboard, Key key, int i)
    {
        if(!enabled) return;
        if(keys.Contains(key))
        {
            keys.Remove(key);
        }

        if(!keysUp.Contains(key))
        {
            keysUp.Add(key);
        }
    }

    private static void MouseDown(IMouse mouse, MouseButton button)
    {
        if(!enabled) return;

        if(!mouseButtons.Contains(button))
        {
            mouseButtonsDown.Add(button);
            mouseButtons.Add(button);
        }
    }

    private static void MouseUp(IMouse mouse, MouseButton button)
    {
        if(!enabled) return;
        if(mouseButtons.Contains(button))
        {
            mouseButtons.Remove(button);
        }

        if(!mouseButtonsUp.Contains(button))
        {
            mouseButtonsUp.Add(button);
        }
    }

    public static void OnUpdate()
    {
        if(!enabled)
        { 
            mouseDelta = new Vector2(0);
            mousePosition = new Vector2(0);
            lastMousePosition = new Vector2(0);
            keysDown.Clear();
            keysUp.Clear();
            keys.Clear();
            return;
        }

        mousePosition = mouse.Position;
        mouseDelta = mousePosition - lastMousePosition;
        lastMousePosition = mousePosition;

        keysDown.Clear();
        keysUp.Clear();
    }

    public static void SetCursorMode(CursorMode cursorMode)
    {
        if(!enabled) return;
        mouse.Cursor.CursorMode = cursorMode;
    }

    public static bool GetKey(Key key)
    {
        if(!enabled) return false;
        return keys.Contains(key);
    }

    public static bool GetKeyDown(Key key)
    {
        if(!enabled) return false;
        return keysDown.Contains(key);
    }

    public static bool GetKeyUp(Key key)
    {
        if(!enabled) return false;
        return keysUp.Contains(key);
    }

    public static bool GetKey(MouseButton key)
    {
        if(!enabled) return false;
        return mouseButtons.Contains(key);
    }

    public static bool GetKeyDown(MouseButton key)
    {
        if(!enabled) return false;
        return mouseButtonsDown.Contains(key);
    }

    public static bool GetKeyUp(MouseButton key)
    {
        if(!enabled) return false;
        return mouseButtonsUp.Contains(key);
    }
}