using Newtonsoft.Json;

namespace IonEngine;

public class Component 
{ 
   
    public GameObject gameObject;
    protected virtual void Start()
    {

    }

    protected virtual void Create()
    {

    }

    protected virtual void EditorStart()
    {

    }

    protected virtual void Update()
    {
        
    }

    protected virtual void EditorUpdate()
    {
        
    }

    protected virtual void Render()
    {
        
    }

    protected virtual void EditorGuiRender()
    {
        
    }

    protected virtual void Delete()
    {
        
    }

    protected virtual void EditorLateUpdate()
    {
        
    }

    protected virtual void LateUpdate()
    {

    }

    public void OnDelete()
    {
        Delete();
    }

    public void OnUpdate()
    {
        Update();
    }

    public void OnEditorLateUpdate()
    {
        EditorLateUpdate();
    }

    public void OnRuntimeLateUpdate()
    {
        LateUpdate();
    }

    public void OnStart()
    {
        Start();
    }

    public void OnCreate()
    {
        Create();
    }

    public void OnRender()
    {
        Render();
    }
    public void OnEditorGuiRender()
    {
        EditorGuiRender();
    }

    public void OnEditorStart()
    {
        EditorStart();
    }

    public void OnEditorUpdate()
    {
        EditorUpdate();
    }
}