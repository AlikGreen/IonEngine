using System.IO;
using Silk.NET.OpenGL;

namespace IonEngine;

public class Shader
{
    uint id;

    public Shader(string path, ShaderType shaderType)
    {
        string shaderSource = File.ReadAllText(path);

        id = Window.gl.CreateShader(shaderType);
        Window.gl.ShaderSource(id, shaderSource);


        Window.gl.CompileShader(id);

        Window.gl.GetShader(id, ShaderParameterName.CompileStatus, out int success);
        if (success == 0)
        {
            string infoLog = Window.gl.GetShaderInfoLog(id);
            Console.WriteLine(infoLog);
        }
    }

    public void Detach(uint program)
    {
        Window.gl.DetachShader(program, id);  
    }

    public void Attach(uint program)
    {
        Window.gl.AttachShader(program, id);  
    }

    public void Delete()
    {
        Window.gl.DeleteShader(id);
    }
}