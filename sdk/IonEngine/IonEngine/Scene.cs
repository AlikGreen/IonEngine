using System.Collections;
using System.ComponentModel;
using Coral.Managed.Interop;

namespace IonEngine;

public class Scene
{
    private IntPtr handle;

    internal Scene(IntPtr handle)
    {
        this.handle = handle;
    }

    public View<T> View<T>()
    {
        unsafe
        {
            ulong typeId = (ulong)typeof(T).TypeHandle.Value.ToInt64();
            
            if(typeof(T).IsDefined(typeof(NativeComponentAttribute), false))
            {
                typeId = NativeBridge.Registry_getTypeHash(typeof(T).FullName);
            }
            else
            {
                Log.Info($"Registering {typeof(T).FullName}");
                NativeBridge.Registry_registerType(handle, typeId, (ulong)sizeof(T), (ulong)sizeof(T));   
            }
                
            NativeArray<ulong> types = new NativeArray<ulong>(1);
            types[0] = typeId;
            
            IntPtr viewHandle = NativeBridge.Registry_createView(handle, types);
            return new View<T>(viewHandle);
        }
    }
}
