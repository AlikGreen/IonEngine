using System.Collections;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace NeonEngine;

[StructLayout(LayoutKind.Sequential)]
internal struct EntityComponentData
{
    public ulong entityId;
    public IntPtr component;
}

public static class ViewInterface
{
#pragma warning disable CS0649
    internal static unsafe delegate*<IntPtr, ulong, EntityComponentData> getAtIndexCall;
    internal static unsafe delegate*<IntPtr, ulong> getSizeCall;
#pragma warning restore CS0649
}

public class View<T>
{
    private IntPtr handle;
    private ulong size;

    public int Length => (int)size;

    internal View(IntPtr handle)
    {
        this.handle = handle;
        unsafe
        {
            size = ViewInterface.getSizeCall(handle);
        }
    }
    
    public delegate void ComponentAction(ulong entity, ref T component);

    public void ForEach(ComponentAction action)
    {
        for (ulong i = 0; i < size; i++)
        {
            unsafe
            {
                EntityComponentData data = ViewInterface.getAtIndexCall(handle, i);
                void** componentArray = (void**)data.component;
                void* actualComponent = componentArray[0];  // First component
            
                action(data.entityId, ref Unsafe.AsRef<T>(actualComponent));
            }
        }
    }
}