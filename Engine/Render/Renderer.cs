using System.Drawing;
using System.Numerics;
using Newtonsoft.Json;
using Silk.NET.Maths;
using Silk.NET.OpenGL;

namespace IonEngine;

public static class Renderer
{
    public static Camera camera;
    public static Color clearColor = Color.CornflowerBlue;
    public static List<DirectionalLight> directionalLights = new List<DirectionalLight>();
    public static List<PointLight> pointLights = new List<PointLight>();
    [JsonIgnore]
    public static Vector2D<uint> renderSize = new Vector2D<uint>(100, 100);
    
    public static List<MeshRenderer> meshRenderers = new List<MeshRenderer>();

    [JsonIgnore]
    private static FrameBuffer frameBuffer;

    [JsonIgnore]
    private static uint vao, vbo, ubo;
    [JsonIgnore]
    private static ShaderProgram screenRendererShader;
    [JsonIgnore]
    private static ShaderProgram debugShader;

    private static List<Line> debugLines = new List<Line>();

    public static unsafe void Init()
    {
        frameBuffer = new FrameBuffer((uint)Window.window.Size.X, (uint)Window.window.Size.Y);

        // Create and bind the VAO
        vao = Window.gl.GenVertexArray();
        Window.gl.BindVertexArray(vao);

        // Create and bind the VBO
        vbo = Window.gl.GenBuffer();
        Window.gl.BindBuffer(BufferTargetARB.ArrayBuffer, vbo);

        // Define the full-screen quad
        float[] vertices = {
            -1.0f,  1.0f,
            -1.0f, -1.0f,
             1.0f, -1.0f,

            -1.0f,  1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f
        };

        // Upload the vertex data to the VBO
        fixed(float* ptr = vertices)
        {
            Window.gl.BufferData(BufferTargetARB.ArrayBuffer, (uint)(vertices.Length * sizeof(float)), ptr, BufferUsageARB.StaticDraw);
        }

        Window.gl.VertexAttribPointer(0, 2, VertexAttribPointerType.Float, false, 2 * sizeof(float), (void*)0);
        Window.gl.EnableVertexAttribArray(0);

        ubo = Window.gl.GenBuffer();
        Window.gl.BindBuffer(BufferTargetARB.ArrayBuffer, ubo);

        float[] uvs = {
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,

            0.0f, 1.0f,
            1.0f, 0.0f,
            1.0f, 1.0f
        };


        // Upload the vertex data to the VBO
        fixed(float* ptr = uvs)
        {
            Window.gl.BufferData(BufferTargetARB.ArrayBuffer, (uint)(uvs.Length * sizeof(float)), ptr, BufferUsageARB.StaticDraw);
        }

        Window.gl.VertexAttribPointer(1, 2, VertexAttribPointerType.Float, false, 2 * sizeof(float), (void*)0);
        Window.gl.EnableVertexAttribArray(1);


        // Create and compile the vertex shader
        Shader sVert = new Shader(Project.AssetsDirectory + "Shaders/texture.vert", ShaderType.VertexShader);
        Shader sFrag = new Shader(Project.AssetsDirectory + "Shaders/texture.frag", ShaderType.FragmentShader);
        screenRendererShader = new ShaderProgram(sVert, sFrag);

        Shader dVert = new Shader(Project.AssetsDirectory + "Shaders/debug.vert", ShaderType.VertexShader);
        Shader dFrag = new Shader(Project.AssetsDirectory + "Shaders/debug.frag", ShaderType.FragmentShader);
        debugShader = new ShaderProgram(dVert, dFrag);

        debugLines.Add(new Line(new Vector3(0, 0, 0), new Vector3(0, 10, 0)));
    }


    public static unsafe uint OnRender(uint width, uint height)
    {     
        frameBuffer.Resize(width, height);
        frameBuffer.StartFrame();

        Window.gl.Enable(EnableCap.DepthTest);

        Window.gl.Enable(EnableCap.Blend);
        Window.gl.BlendFunc(BlendingFactor.SrcAlpha, BlendingFactor.OneMinusSrcAlpha);

        camera.size = new Vector2(renderSize.X, renderSize.Y);

        if(camera == null)
        {
            Console.WriteLine("Camera Null");
            Window.gl.ClearColor(Color.Black);
            Window.gl.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);   
            return 0;
        }
        
        Window.gl.ClearColor(clearColor);
        Window.gl.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);   

        foreach (MeshRenderer meshRenderer in meshRenderers)
        {    
            if(meshRenderer.mesh == null || meshRenderer.material == null)
                continue;

            Matrix4x4 model = Matrix4x4.CreateScale(meshRenderer.gameObject.transform.scale) * 
                              Matrix4x4.CreateFromYawPitchRoll(Maths.DegreesToRadians(meshRenderer.gameObject.transform.rotation.X), Maths.DegreesToRadians(meshRenderer.gameObject.transform.rotation.Y), Maths.DegreesToRadians(meshRenderer.gameObject.transform.rotation.Z)) *
                              Matrix4x4.CreateTranslation(meshRenderer.gameObject.transform.position);

                

            Material.shaders[meshRenderer.material.shaderIndex].Use();
            meshRenderer.material.SetShaderProperties();

            Material.shaders[meshRenderer.material.shaderIndex].SetUniform("camPos", camera.gameObject.transform.position); 

            Material.shaders[meshRenderer.material.shaderIndex].SetUniform("ambientLightColor", Lighting.ambientColor * Lighting.ambientIntensity); 

            for (int i = 0; i < directionalLights.Count; i++)
            {
                directionalLights[i].SetUniforms(Material.shaders[meshRenderer.material.shaderIndex], i);
            }
            Material.shaders[meshRenderer.material.shaderIndex].SetUniform("numDirectionalLights", directionalLights.Count); 

            for (int i = 0; i < pointLights.Count; i++)
            {
                pointLights[i].SetUniforms(Material.shaders[meshRenderer.material.shaderIndex], i);
            }
            Material.shaders[meshRenderer.material.shaderIndex].SetUniform("numPointLights", pointLights.Count); 

            Material.shaders[meshRenderer.material.shaderIndex].SetUniform("model", model);
            Material.shaders[meshRenderer.material.shaderIndex].SetUniform("view", camera.view);
            Material.shaders[meshRenderer.material.shaderIndex].SetUniform("projection", camera.projection);

            meshRenderer.mesh.Draw();
            Window.gl.BindVertexArray(0);
        } 

        float[] vertices = new float[debugLines.Count * 6];

        for (int i = 0; i < debugLines.Count; i++)
        {
            vertices[(i*6)+0] = debugLines[i].startPos.X;
            vertices[(i*6)+1] = debugLines[i].startPos.Y;
            vertices[(i*6)+2] = debugLines[i].startPos.Z;
            vertices[(i*6)+3] = debugLines[i].endPos.X;
            vertices[(i*6)+4] = debugLines[i].endPos.Y;
            vertices[(i*6)+5] = debugLines[i].endPos.Z;
        }

        debugShader.Use();

        debugShader.SetUniform("view", camera.view);
        debugShader.SetUniform("projection", camera.projection);

        uint vbo; // Vertex Buffer Object
        Window.gl.GenBuffers(1, out vbo);
        Window.gl.BindBuffer(BufferTargetARB.ArrayBuffer, vbo);
        fixed(float* ptr = vertices)
        {
            Window.gl.BufferData(GLEnum.ArrayBuffer, (uint)(vertices.Length * sizeof(float)), ptr, BufferUsageARB.StaticDraw);
        }

        Window.gl.BindBuffer(BufferTargetARB.ArrayBuffer, vbo);
        Window.gl.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, 0, (void*)0);
        Window.gl.EnableVertexAttribArray(0);

        Window.gl.DrawArrays(PrimitiveType.Lines, 0, (uint)vertices.Length);

        Window.gl.DisableVertexAttribArray(0);
        Window.gl.BindBuffer(BufferTargetARB.ArrayBuffer, 0);

        frameBuffer.EndFrame();

        return frameBuffer.texture.id;
    }

    public static unsafe void DrawFrameBufferToScreen()
    {
        screenRendererShader.Use();
        Window.gl.BindVertexArray(vao);
        Window.gl.BindBuffer(BufferTargetARB.ArrayBuffer, vbo);

        frameBuffer.texture.Bind();

        // Bind the VAO and draw the full-screen quad
        Window.gl.BindVertexArray(vao);
        Window.gl.DrawArrays(PrimitiveType.Triangles, 0, 6);
    }

    public static void Dispose()
    {
        Window.gl.BindBuffer(BufferTargetARB.ArrayBuffer, 0);

        foreach (MeshRenderer meshRenderer in meshRenderers)
        {    
            meshRenderer.material.Dispose();
            meshRenderer.mesh?.Dispose();
        }

        meshRenderers = new List<MeshRenderer>();
        directionalLights = new List<DirectionalLight>();
        pointLights = new List<PointLight>();
    }
}

public struct Line
{
    public Vector3 startPos;
    public Vector3 endPos;

    public Line(Vector3 start, Vector3 end)
    {
        startPos = start;
        endPos = end;
    }
}