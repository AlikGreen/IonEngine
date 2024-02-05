using System.IO;
using Silk.NET.OpenGL;
using StbImageSharp;

namespace IonEngine;

public class Texture
{
    public uint id;
    public uint width, height;
    public unsafe Texture(string path)
    {
        id = Window.gl.GenTexture();
        Window.gl.BindTexture(TextureTarget.Texture2D, id);


        Window.gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapS, (int)TextureWrapMode.Repeat);
        Window.gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapT, (int)TextureWrapMode.Repeat);
        Window.gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMinFilter, (int)TextureMinFilter.Linear);
        Window.gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMagFilter, (int)TextureMagFilter.Linear);

        Window.gl.GenerateMipmap(GLEnum.Texture2D);

        StbImage.stbi_set_flip_vertically_on_load(1);

        // Load the image with minimal memory usage to check the number of channels.
        ImageResult imageInfo = ImageResult.FromStream(File.OpenRead(path), ColorComponents.Grey);
        int numChannels = (int)imageInfo.Comp;

        // Load the image again with the correct color components.
        ImageResult image;
        if (numChannels == 3)
        {
            image = ImageResult.FromStream(File.OpenRead(path), ColorComponents.RedGreenBlue);
        }
        else // numChannels == 4
        {
            image = ImageResult.FromStream(File.OpenRead(path), ColorComponents.RedGreenBlueAlpha);
        }

        width = (uint)image.Width;
        height = (uint)image.Height;

        fixed (byte* ptr = image.Data)
        {  
            Window.gl.TexImage2D(GLEnum.Texture2D, 0, (int)InternalFormat.Rgba, width, height, 0, PixelFormat.Rgba, PixelType.UnsignedByte, ptr);
        }
        Unbind();
    }

    public unsafe Texture(uint width, uint height)
    {
        this.width = width;
        this.height = height;
        id = Window.gl.GenTexture();
        Window.gl.BindTexture(TextureTarget.Texture2D, id);

        Window.gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapS, (int)TextureWrapMode.Repeat);
        Window.gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapT, (int)TextureWrapMode.Repeat);
        Window.gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMinFilter, (int)TextureMinFilter.Linear);
        Window.gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMagFilter, (int)TextureMagFilter.Linear);

        Window.gl.GenerateMipmap(GLEnum.Texture2D);

        Window.gl.TexImage2D(TextureTarget.Texture2D, 0, (int)InternalFormat.Rgba, width, height, 0, PixelFormat.Rgba, PixelType.UnsignedByte, null);
        Unbind();
    }


    public void Bind(TextureUnit textureUnit = TextureUnit.Texture0)
    {
        Window.gl.ActiveTexture(textureUnit);
        Window.gl.BindTexture(TextureTarget.Texture2D, id);
        Window.gl.Enable(EnableCap.Texture2D);
    }

    public void Dispose()
    {
        Unbind();
        Window.gl.DeleteTexture(id);
    }

    public void Unbind()
    {
        Window.gl.BindTexture(TextureTarget.Texture2D, 0);
    }
}