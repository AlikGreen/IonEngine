using Newtonsoft.Json;

namespace IonEngine;

public class Scene
{
    public List<GameObject> gameObjects = new List<GameObject>();

    private bool runtimeStarted = false;
    private bool editorStarted = false;

    public void AddGameObject(GameObject gameObject)
    {
        gameObjects.Add(gameObject);

        if(runtimeStarted)
            gameObject.OnStart();

        if(editorStarted)
            gameObject.OnEditorStart();
    }

    public void OnRuntimeStart()
    {
        for (int i = 0; i < gameObjects.Count; i++)
        {
            gameObjects[i].OnStart();
            if(!gameObjects[i].created)
                gameObjects[i].OnCreate();
        }

        runtimeStarted = true;
    }

    public void OnEditorStart()
    {
        for (int i = 0; i < gameObjects.Count; i++)
        {
            gameObjects[i].OnEditorStart();
            if(!gameObjects[i].created)
                gameObjects[i].OnCreate();
        }
        
        editorStarted = true;
    }
    public void OnRuntimeUpdate()
    {
        for (int i = 0; i < gameObjects.Count; i++)
        {
            gameObjects[i].OnUpdate();
        }
    }

    public void OnRuntimeLateUpdate()
    {
        for (int i = 0; i < gameObjects.Count; i++)
        {
            gameObjects[i].OnRuntimeLateUpdate();
        }
    }

    public void OnEditorUpdate()
    {
        for (int i = 0; i < gameObjects.Count; i++)
        {
            gameObjects[i].OnEditorUpdate();
        }
    }

    public void OnEditorLateUpdate()
    {
        for (int i = 0; i < gameObjects.Count; i++)
        {
            gameObjects[i].OnEditorLateUpdate();
        }
    }


    public void OnRender()
    {
        foreach (GameObject gameObject in gameObjects)
        {
            gameObject.OnRender();
        }
    }

    public void OnEditorGuiRender()
    {
        foreach (GameObject gameObject in gameObjects)
        {
            gameObject.OnEditorGuiRender();
        }
    }

    public void OnDelete()
    {
        foreach (GameObject gameObject in gameObjects)
        {
            gameObject.OnDelete();
        }
        gameObjects.Clear();
        runtimeStarted = false;
        editorStarted = false;

        Renderer.Dispose();
    }
}