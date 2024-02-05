using Silk.NET.OpenGL;

namespace IonEngine;

public class FrameBuffer
{
    public Texture texture;
    public uint id;
    public uint depth; // Depth renderbuffer

    public FrameBuffer(uint width, uint height)
    {
        id = Window.gl.GenFramebuffer();
        Window.gl.BindFramebuffer (FramebufferTarget.Framebuffer, id);
        texture = new Texture(width, height);
        Window.gl.FramebufferTexture2D (FramebufferTarget.Framebuffer, FramebufferAttachment.ColorAttachment0, TextureTarget.Texture2D, texture.id, 0);

        // Create and attach depth renderbuffer
        depth = Window.gl.GenRenderbuffer();
        Window.gl.BindRenderbuffer(RenderbufferTarget.Renderbuffer, depth);
        Window.gl.RenderbufferStorage(RenderbufferTarget.Renderbuffer, InternalFormat.DepthComponent, width, height);
        Window.gl.FramebufferRenderbuffer(FramebufferTarget.Framebuffer, FramebufferAttachment.DepthAttachment, RenderbufferTarget.Renderbuffer, depth);

        CheckFramebufferStatus();
        Window.gl.BindFramebuffer (FramebufferTarget.Framebuffer, 0);
    }

    public void StartFrame()
    {
        Window.gl.BindFramebuffer (FramebufferTarget.Framebuffer, id);
        CheckFramebufferStatus();
    }

    public void Resize(uint width, uint height)
    {
        Window.gl.BindFramebuffer (FramebufferTarget.Framebuffer, id);
        if(width != texture.width || height != texture.height)
        {
            texture.Dispose();
            Window.gl.DeleteRenderbuffers(1, in depth); // Dispose of the old depth renderbuffer
            texture = new Texture(width, height);
            Window.gl.FramebufferTexture2D (FramebufferTarget.Framebuffer, FramebufferAttachment.ColorAttachment0, TextureTarget.Texture2D, texture.id, 0);

            // Create and attach new depth renderbuffer
            depth = Window.gl.GenRenderbuffer();
            Window.gl.BindRenderbuffer(RenderbufferTarget.Renderbuffer, depth);
            Window.gl.RenderbufferStorage(RenderbufferTarget.Renderbuffer, InternalFormat.DepthComponent, width, height);
            Window.gl.FramebufferRenderbuffer(FramebufferTarget.Framebuffer, FramebufferAttachment.DepthAttachment, RenderbufferTarget.Renderbuffer, depth);

            CheckFramebufferStatus();
        }
        Window.gl.Viewport(0, 0, width, height); // Adjust the viewport
    }

    public void EndFrame()
    {
        Window.gl.BindFramebuffer (FramebufferTarget.Framebuffer, 0);
    }

    private void CheckFramebufferStatus()
    {
        GLEnum status = Window.gl.CheckFramebufferStatus(GLEnum.Framebuffer);
        if (status != GLEnum.FramebufferComplete)
        {
            Console.WriteLine("Error with framebuffer: " + status);
        }
    }
}
