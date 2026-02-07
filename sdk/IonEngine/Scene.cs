using System.Collections;
using System.ComponentModel;
using Coral.Managed.Interop;

namespace IonEngine;

public class Scene
{
#pragma warning disable CS0649
    internal static unsafe delegate*<IntPtr, NativeArray<ulong>, IntPtr> createViewCall;
    internal static unsafe delegate*<IntPtr, ulong, ulong, ulong, void> registerTypeCall;
    internal static unsafe delegate*<NativeString, ulong> getTypeHash;
#pragma warning restore CS0649

    
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
                typeId = getTypeHash(typeof(T).FullName);
            }
            else
            {
                Log.Info($"Registering {typeof(T).FullName}");
                registerTypeCall(handle, typeId, (ulong)sizeof(T), (ulong)sizeof(T));   
            }
                
            NativeArray<ulong> types = new NativeArray<ulong>(1);
            types[0] = typeId;
            
            IntPtr viewHandle = createViewCall(handle, types);
            return new View<T>(viewHandle);
        }
    }
}
