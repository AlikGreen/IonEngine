using System.Numerics;
using Silk.NET.OpenGL;

namespace IonEngine;

public class ShaderProgram
{
    uint id;

    public ShaderProgram(params Shader[] shaders)
    {
        id = Window.gl.CreateProgram();

        foreach (Shader shader in shaders)
        {
            shader.Attach(id);
        }

        Window.gl.LinkProgram(id);

        Window.gl.GetProgram(id, GLEnum.LinkStatus, out int success);
        if (success == 0)
        {
            string infoLog = Window.gl.GetProgramInfoLog(id);
            Console.WriteLine(infoLog);
        }

        foreach (Shader shader in shaders)
        {
            shader.Detach(id);
            shader.Delete();
        }
    }

    public void Use()
    {
        Window.gl.UseProgram(id);
    }

    private bool disposedValue = false;

    protected virtual void Dispose(bool disposing)
    {
        if (!disposedValue)
        {
            Window.gl.DeleteProgram(id);

            disposedValue = true;
        }
    }

    ~ShaderProgram()
    {
        if (disposedValue == false)
        {
            Console.WriteLine("GPU Resource leak! Did you forget to call Dispose()?");
        }
    }


    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    public void SetUniform(string name, float value)
    {
        int location = Window.gl.GetUniformLocation(id, name);

        if(location == -1)
        {
            Console.WriteLine("Couldent find location of " + name);
            return;
        }

        Window.gl.Uniform1(location, value);
    }

    public void SetUniform(string name, int value)
    {
        int location = Window.gl.GetUniformLocation(id, name);

        if(location == -1)
        {
            Console.WriteLine("Couldent find location of " + name);
            return;
        }

        Window.gl.Uniform1(location, value);
    }

    public void SetUniform(string name, Vector3 value)
    {
        int location = Window.gl.GetUniformLocation(id, name);

        if(location == -1)
        {
            Console.WriteLine("Couldent find location of " + name);
            return;
        }

        Window.gl.Uniform3(location, value);
    }

    public unsafe void SetUniform(string name, Matrix4x4 value)
    {
        int location = Window.gl.GetUniformLocation(id, name);
    
        if(location == -1)
        {
            Console.WriteLine("Couldent find location of " + name);
            return;
        }
    
        float[] matrixElements =
        {
            value.M11, value.M21, value.M31, value.M41,
            value.M12, value.M22, value.M32, value.M42,
            value.M13, value.M23, value.M33, value.M43,
            value.M14, value.M24, value.M34, value.M44
        };
    
        fixed (float* ptr = matrixElements)
        {
            Window.gl.UniformMatrix4(location, 1, false, ptr);
        }
    }

    public void SetUniform(string name, bool value)
    {
        int location = Window.gl.GetUniformLocation(id, name);

        if(location == -1)
        {
            Console.WriteLine("Couldent find location of " + name);
            return;
        }

        Window.gl.Uniform1(location, value ? 1 : 0);
    }
}