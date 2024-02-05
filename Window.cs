using System.Drawing;
using Newtonsoft.Json;
using Silk.NET.Input;
using Silk.NET.Maths;
using Silk.NET.OpenGL;
using Silk.NET.Windowing;

namespace IonEngine;

public class Window
{
    public static IWindow window;
    public static GL gl;
    public static Scene scene = new Scene();
    public static IInputContext inputContext;
    public static Camera camera;
    public static PhysicsWorld physicsWorld = new PhysicsWorld();
    public Window(int width, int height, string title)
    { 
        WindowOptions options = WindowOptions.Default;
        options.Size = new Vector2D<int>(width, height);
        options.Title = title;
        window = Silk.NET.Windowing.Window.Create(options);

        window.Load += OnLoad;
        window.Render += OnRender;
        window.Update += OnUpdate;
        window.Resize += OnResize;
    }

    public void Run()
    {
        window.Run();
    }
    
    protected unsafe virtual void OnLoad()
    {

    }

    protected unsafe virtual void OnUpdate(double dt)
    {

    }

    protected unsafe virtual void OnRender(double time)
    {

    }

    protected unsafe virtual void OnResize(Vector2D<int> size)
    {

    }

    public static void LoadNewScene(List<GameObject> gameObjects, bool editor)
    {
        scene.OnDelete();
        scene.gameObjects.AddRange(gameObjects);

        if(editor)
            scene.OnEditorStart();
        else
            scene.OnRuntimeStart();
    }

    public static void LoadNewScene(string fileName, bool editor)
    {
        string fileText = File.ReadAllText(fileName);
        JsonSerializerSettings settings = new JsonSerializerSettings
        {
            PreserveReferencesHandling = PreserveReferencesHandling.All,
            TypeNameHandling = TypeNameHandling.All
        };
        LoadNewScene(JsonConvert.DeserializeObject<Scene>(fileText, settings).gameObjects, editor);
    }

    public static void SaveScene(string fileName)
    {
        JsonSerializerSettings settings = new JsonSerializerSettings
        {
            PreserveReferencesHandling = PreserveReferencesHandling.All,
            TypeNameHandling = TypeNameHandling.All
        };
        string fileText = JsonConvert.SerializeObject(scene, settings);
        File.WriteAllText(fileName, fileText);
    }
}