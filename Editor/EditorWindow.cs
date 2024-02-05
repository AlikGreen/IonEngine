using System.Drawing;
using System.Numerics;
using IonEngine;
using Silk.NET.Input;
using Silk.NET.OpenGL;
using Silk.NET.Windowing;

namespace IonEditor;

public class EditorWindow : IonEngine.Window
{
    Editor editor;
    GameObject camObj;
    public EditorWindow(int width, int height, string title) : base(width, height, title)
    {
    }

    protected unsafe override void OnLoad()
    {
        window.Initialize();
        gl = GL.GetApi(window);
        window.MakeCurrent();
        Renderer.Init();
        inputContext = window.CreateInput();
        Input.Init(inputContext);

        camObj = new GameObject("Camera");
        camObj.AddComponent(new Camera());
        camObj.AddComponent(new EditorCamera());
        camObj.transform.rotation.X = 1;
        camObj.transform.position = new Vector3(0f, 0, -1f);

        

        editor = new Editor();

        scene.OnEditorStart();
        camObj.OnEditorStart();
    }

    protected unsafe override void OnUpdate(double dt)
    {
        Time.OnUpdate((float)dt);
        scene.OnEditorUpdate();
        camObj.OnEditorUpdate();

        scene.OnEditorLateUpdate();


        Input.OnUpdate();

    }

    protected unsafe override void OnRender(double time)
    {
        gl.ClearColor(Color.White);
        gl.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);  

        editor.Render((float)time);
        camObj.OnRender();
        scene.OnRender();
        scene.OnEditorGuiRender();
    }
}