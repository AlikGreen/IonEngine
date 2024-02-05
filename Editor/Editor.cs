using System.Drawing;
using System.Numerics;
using System.Reflection;
using System.Text;
using ImGuiNET;
using Silk.NET.OpenGL.Extensions.ImGui;

using IonEngine;
using System.Runtime.InteropServices;
using Newtonsoft.Json;

namespace IonEditor;

public class Editor
{
    private ImGuiController controller = null;
    InspectorPanel inspectorPanel = new InspectorPanel();
    ContentBrowserPanel contentBrowserPanel = new ContentBrowserPanel();
    HierachyPanel hierachyPanel = new HierachyPanel();
    ViewportPanel viewportPanel = new ViewportPanel();
    ObjectEditorPanel objectEditorPanel = new ObjectEditorPanel();
    MainMenuBar mainMenuBar = new MainMenuBar();
    public Editor()
    {
        controller = new ImGuiController
        (
            Window.gl,
            Window.window,
            Window.inputContext
        );

        ImGuiIOPtr io = ImGui.GetIO();
        io.ConfigFlags |= ImGuiConfigFlags.DockingEnable;

        Debug.Fatal("Ahh");
    }

    public void PushStyles()
    {
        ImGui.PushStyleVar(ImGuiStyleVar.FrameRounding, 4);
        ImGui.PushStyleVar(ImGuiStyleVar.TabRounding, 4);
        ImGui.PushStyleVar(ImGuiStyleVar.WindowRounding, 4);
        ImGui.PushStyleVar(ImGuiStyleVar.GrabRounding, 4);

        ImGui.PushStyleVar(ImGuiStyleVar.FramePadding, new Vector2(6, 5));
        ImGui.PushStyleVar(ImGuiStyleVar.CellPadding, new Vector2(6, 5));
        ImGui.PushStyleVar(ImGuiStyleVar.WindowPadding, 10);
        ImGui.PushStyleVar(ImGuiStyleVar.DockingSeparatorSize, 3);

        ImGui.PushStyleVar(ImGuiStyleVar.ItemSpacing, new Vector2(10, 5));
        ImGui.PushStyleVar(ImGuiStyleVar.ItemInnerSpacing, new Vector2(8, 5));

        ImGui.PushStyleVar(ImGuiStyleVar.ScrollbarSize, 15);
    }

    public void PopStyles()
    {
        ImGui.PopStyleVar(11);
    }

    public unsafe void Render(float time)
    {
        controller.Update(time);
        
        PushStyles();

        Window.gl.ClearColor(Color.Black);
        // I didnt want to use Silk.Net open gl in this file because it conflicted with something? maybe
        Window.gl.Clear(16384); // 16384 == ColorBufferBit i think

        ImGui.DockSpaceOverViewport();

        mainMenuBar.Draw();
        
        viewportPanel.Draw();

        int selectedObj = hierachyPanel.Draw();

        inspectorPanel.Draw(selectedObj);

        string filePath = contentBrowserPanel.Draw();
        if(File.Exists(filePath))
        {
            try
            {
                string fileText = File.ReadAllText(filePath);
                JsonSerializerSettings settings = new JsonSerializerSettings
                {
                    PreserveReferencesHandling = PreserveReferencesHandling.All,
                    TypeNameHandling = TypeNameHandling.All
                };
                object obj = JsonConvert.DeserializeObject(fileText, settings);
                objectEditorPanel.obj = obj;
                objectEditorPanel.filePath = filePath;
            }
            catch(Exception e)
            {
                Console.WriteLine(e);
            }
        }

        objectEditorPanel.Draw();

        

        ImGui.ShowStyleEditor();

        
        controller.Render();
    }
}