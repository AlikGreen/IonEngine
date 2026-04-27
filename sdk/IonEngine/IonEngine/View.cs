using System.Collections;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IonEngine;

[StructLayout(LayoutKind.Sequential)]
internal struct EntityComponentData
{
    public ulong entityId;
    public IntPtr component;
}

public class View<T>
{
    private IntPtr handle;
    private ulong size;

    public int length => (int)size;

    internal View(IntPtr handle)
    {
        this.handle = handle;
        unsafe
        {
            size = NativeBridge.View_getSize(handle);
        }
    }
    
    public delegate void ComponentAction(ulong entity, ref T component);

    public void ForEach(ComponentAction action)
    {
        for (ulong i = 0; i < size; i++)
        {
            unsafe
            {
                EntityComponentData data = NativeBridge.View_getAtIndex(handle, i);
                void** componentArray = (void**)data.component;
                void* actualComponent = componentArray[0];  // First component
            
                action(data.entityId, ref Unsafe.AsRef<T>(actualComponent));
            }
        }
    }
}