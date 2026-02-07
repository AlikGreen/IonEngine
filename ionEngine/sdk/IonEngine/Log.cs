using Coral.Managed.Interop;

namespace IonEngine;

public static class Log
{
#pragma warning disable CS0649
    internal static unsafe delegate*<NativeString, void> logInfoCall;
#pragma warning restore CS0649

    public static void Info(string text)
    {
        unsafe
        {
            logInfoCall(text);
        }
    }
}