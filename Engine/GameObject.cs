using System.Text.Json.Serialization;

namespace IonEngine;

public class GameObject 
{
    public string name;
    public List<Component> components = new List<Component>();
    public Transform transform = new Transform();

    [JsonIgnore]
    private bool runtimeStarted = false;
    [JsonIgnore]
    private bool editorStarted = false;

    [JsonIgnore]
    public bool created {get; private set;} = false;

    public GameObject(string name) {
        this.name = name;
    }

    public GameObject() 
    {
        name = "";
    }

    public T GetComponent<T>() where T : Component
    {
        foreach (Component component in components)
        {
            if (component is T)
            {
                return component as T;
            }
        }
        return null;
    }

    public bool HasComponent<T>() where T : Component
    {
        foreach (Component component in components)
        {
            if (component is T)
            {
                return true;
            }
        }
        return false;
    }

    public bool RemoveComponent<T>() where T : Component
    {
        foreach (Component component in components)
        {
            if (component is T)
            {
                components.Remove(component);
                return true;
            }
        }
        return false;
    }

    public void AddComponent(Component component) 
    {
        components.Add(component);
        component.gameObject = this;

        if(runtimeStarted)
            component.OnStart();

        if(editorStarted)
            component.OnEditorStart();

        component.OnCreate();
    }

    public void OnStart() 
    {
        foreach (Component component in components) 
        {
            Console.WriteLine("Started at" + transform.position);
            component.OnStart();
        }
        runtimeStarted = true;
    }

    public void OnCreate() 
    {
        foreach (Component component in components) 
        {
            Console.WriteLine("Created at" + transform.position);
            component.OnCreate();
        }
        created = true;
    }

    public void OnUpdate() 
    {
        foreach (Component component in components) 
        {
            component.OnUpdate();
        }
    }

    public void OnRender() 
    {
        foreach (Component component in components) 
        {
            component.OnRender();
        }
    }

    public void OnEditorUpdate() 
    {
        foreach (Component component in components) 
        {
            component.OnEditorUpdate();
        }
    }

    public void OnRuntimeLateUpdate()
    {
       foreach (Component component in components) 
        {
            component.OnRuntimeLateUpdate();
        } 
    }

    public void OnEditorLateUpdate()
    {

        foreach (Component component in components) 
        {
            component.OnEditorLateUpdate();
        }
    }

    public void OnEditorStart() 
    {
        foreach (Component component in components) 
        {
            //component.gameObject = this;
            component.OnEditorStart();
        }
        editorStarted = true;
    }

    public void OnEditorGuiRender() 
    {
        foreach (Component component in components) 
        {
            component.OnEditorGuiRender();
        }
    }

    public void OnDelete() 
    {
        foreach (Component component in components) 
        {
            component.OnDelete();
        }
    }
}