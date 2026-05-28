using Coral.Managed.Interop;
using IonEngine.Maths;

namespace IonEngine;

public struct Vertex
{
    public Vertex(Vector3 position, Vector3 normal, Vector2 uv)
    {
        this.position = position;
        this.normal = normal;
        this.uv = uv;
    }
    
    public Vertex(Vector3 position, Vector3 normal)
    {
        this.position = position;
        this.normal = normal;
    }
    
    public Vertex(Vector3 position)
    {
        this.position = position;
    }
    
    public Vector3 position;
    public Vector3 normal;
    public Vector2 uv;
}

public class Mesh
{
    public Mesh()
    {
        unsafe
        {
            id = NativeBridge.Mesh_create();
            ptr = NativeBridge.Mesh_get(id);
        }
    }

    internal Mesh(uint id)
    {
        this.id = id;
        unsafe
        {
            ptr = NativeBridge.Mesh_get(id);
        }
    }

    ~Mesh()
    {
        unsafe
        {
            NativeBridge.Mesh_release(id);   
        }
    }
    
    internal IntPtr ptr;
    internal uint id;

    public Vertex[] vertices
    {
        get
        {
            unsafe
            {
                return NativeBridge.Mesh_getVertices(ptr);
            }
        }
        set
        {
            unsafe
            {
                NativeBridge.Mesh_setVertices(ptr, value);
            }
        }
    }
    
    public int[] indices
    {
        get
        {
            unsafe
            {
                return NativeBridge.Mesh_getIndices(ptr);
            }
        }
        set
        {
            unsafe
            {
                NativeBridge.Mesh_setIndices(ptr, value);
            }
        }
    }
}