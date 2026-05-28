namespace IonEngine;

public class Material
{
    public Material()
    {
        unsafe
        {
            id = NativeBridge.MaterialInstance_create();
            ptr = NativeBridge.MaterialInstance_get(id);
        }
    }

    internal Material(uint id)
    {
        this.id = id;
        unsafe
        {
            ptr = NativeBridge.MaterialInstance_get(id);
        }
    }

    ~Material()
    {
        unsafe
        {
            NativeBridge.MaterialInstance_release(id);   
        }
    }
    
    internal IntPtr ptr;
    internal uint id;
}