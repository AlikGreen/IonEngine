using System.Runtime.CompilerServices;
using Coral.Managed.Interop;

namespace IonEngine;

public struct Entity
{
    internal Entity(ulong id, IntPtr registry)
    {
        this.id = id;
        this.registry = registry;
    }
    
    private readonly ulong id;
    private readonly IntPtr registry;

    public ref T Get<T>()
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
                NativeBridge.Registry_registerType(registry, typeId, (ulong)sizeof(T), (ulong)sizeof(T));   
            }

            return ref Unsafe.AsRef<T>(NativeBridge.Registry_getEntityComponent(registry, id, typeId).ToPointer());
        }
    }
}