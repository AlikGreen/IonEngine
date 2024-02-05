using System.Numerics;
using ImGuiNET;
using IonEngine;

namespace IonEditor;

public class HierachyPanel
{
    private int selectedObj = -1;
    public int Draw()
    {
        ImGui.Begin("Create GameObject", ImGuiWindowFlags.NoCollapse);
        if (ImGui.IsWindowHovered() && ImGui.IsMouseReleased(ImGuiMouseButton.Right))
        {
            ImGui.OpenPopup("GameObjects");
        }

        if (ImGui.BeginPopup("GameObjects"))
        {
            if (ImGui.Selectable("Create Empty"))
            {
                Window.scene.AddGameObject(new GameObject("Empty"));
            }
            if (ImGui.Selectable("Create Cube"))
            {
                GameObject gameObject = new GameObject("Cube");
                gameObject.AddComponent(new MeshRenderer(Project.AssetsDirectory + "/Meshes/cube.obj", Project.AssetsDirectory + "/Materials/default.mat"));
                Window.scene.AddGameObject(gameObject);
            }
            ImGui.EndPopup();
        }
        for (int i = 0; i < Window.scene.gameObjects.Count; i++)
        {
            ImGui.PushID(i);
            if(ImGui.Selectable(Window.scene.gameObjects[i].name))
            {
                selectedObj = i;
            }

            if (ImGui.IsItemHovered() && ImGui.IsMouseReleased(ImGuiMouseButton.Right))
            {
                ImGui.OpenPopup("options");
            }

            if (ImGui.BeginPopup("options"))
            {
                if (ImGui.Selectable("Delete"))
                {
                    Window.scene.gameObjects[i].OnDelete();
                    Window.scene.gameObjects.RemoveAt(i);
                    if(selectedObj == i) selectedObj = -1;
                    i--;
                }
                ImGui.EndPopup();
            }
            ImGui.PopID();

        }
        ImGui.End();

        return selectedObj;
    }
}