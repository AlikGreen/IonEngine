namespace IonEngine;

public abstract class ComponentScript
{
    protected ComponentScript()
    {
        entity = new Entity(0, IntPtr.Zero);
    }

    internal ComponentScript(ulong entityId, IntPtr registry)
    {
        entity = new Entity(entityId, registry);
    }
    
    public Entity entity { get; internal set; }
    public ref Transform transform => ref entity.Get<Transform>();
    public ref Tag tag => ref entity.Get<Tag>();

    
    public virtual void Start() { }
    public virtual void Update() { }
}