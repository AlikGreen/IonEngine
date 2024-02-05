using System.Drawing;
using System.Numerics;
using Silk.NET.Input;
using Silk.NET.Maths;
using Silk.NET.OpenGL;
using Silk.NET.Windowing;

namespace IonEngine;

class GameWindow : Window
{
    public GameWindow(int width, int height, string title) : base(width, height, title)
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

        LoadNewScene(Project.AssetsDirectory + "Scenes/main.scion", false);

        scene.OnRuntimeStart();
    }

    protected unsafe override void OnUpdate(double dt)
    {
        Time.OnUpdate((float)dt);

        scene.OnRuntimeUpdate();
        physicsWorld.Update();
        scene.OnRuntimeLateUpdate();

        Input.OnUpdate();
    }

    protected unsafe override void OnRender(double time)
    {
        Renderer.OnRender((uint)window.Size.X, (uint)window.Size.Y);
        gl.ClearColor(Color.White);
        gl.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);  

        Renderer.DrawFrameBufferToScreen();
        scene.OnRender();
    }
}
