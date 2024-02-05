using System.Numerics;
using System.Runtime.InteropServices;
using ImGuiNET;
using IonEngine;

namespace IonEditor;

public class ViewportPanel
{
    public unsafe void Draw()
    {
        ImGui.Begin("Viewport", ImGuiWindowFlags.NoCollapse);
        if(ImGui.IsWindowFocused())
        {
            Input.enabled = true;
        }
        else
        {
            Input.enabled = false;
        }
        Vector2 size = ImGui.GetContentRegionAvail();
        ImGui.Image((nint)Renderer.OnRender((uint)size.X, (uint)size.Y), size, new Vector2(0, 1), new Vector2(1, 0));

        if(ImGui.BeginDragDropTarget())
        {
            ImGuiPayloadPtr payload = ImGui.AcceptDragDropPayload("CONTENT_BROWSER_ITEM");
            if(payload.NativePtr != null)
            {
                string droppedFile = Marshal.PtrToStringUTF8((IntPtr)payload.Data.ToPointer(), payload.DataSize);
        
                Console.WriteLine(droppedFile);
                string extension = Path.GetExtension(droppedFile);


                if(extension == ".scion")
                {
                    Window.LoadNewScene(droppedFile, true);
                }
            }
            ImGui.EndDragDropTarget();
        }

        Window.gl.Viewport(Window.window.Size);
        ImGui.End();
    }
}