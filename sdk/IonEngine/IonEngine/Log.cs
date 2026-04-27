using Coral.Managed.Interop;

namespace IonEngine;

public static class Log
{
    public static void Info(string text)
    {
        unsafe
        {
            NativeBridge.Log_logInfo(text);
        }
    }
}