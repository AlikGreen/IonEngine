using ImGuiNET;
using IonEngine;

namespace IonEditor;

public class MainMenuBar
{
    private FileSystemDialog dialog = new FileSystemDialog();

    private string currentFilePath = Project.AssetsDirectory;
    public void Draw()
    {
         ImGui.BeginMainMenuBar();
        if (ImGui.BeginMainMenuBar()) 
        {
            if (ImGui.BeginMenu("File")) 
            {
                if (ImGui.MenuItem("New")) 
                { 
                    Window.scene.OnDelete();
                    Window.scene = new Scene();

                    Window.scene.OnEditorStart();
                }
                if (ImGui.MenuItem("Open", "Ctrl+O")) 
                { 
                    if (dialog.ShowFileDialog(currentFilePath, "scion"))
                    {
                        Window.LoadNewScene(dialog.FileName, true);
                        currentFilePath = dialog.FolderName;
                    }
                }
                if (ImGui.MenuItem("Save", "Ctrl+S"))
                {
                    if(currentFilePath != "" && Path.GetExtension(currentFilePath) == "scion")
                    {
                        Window.SaveScene(currentFilePath);
                    }   
                    else if (dialog.ShowSaveFileDialog(currentFilePath, "scion"))
                    {
                        Window.SaveScene(dialog.FileName);
                        currentFilePath = dialog.FileName;
                    }
                }
                if (ImGui.MenuItem("Save as..")) 
                {
                    if (dialog.ShowSaveFileDialog(currentFilePath, "scion"))
                    {
                        Window.SaveScene(dialog.FileName);
                        currentFilePath = dialog.FileName;
                    }
                }
            ImGui.EndMenu();
            }
            ImGui.EndMainMenuBar();
        }

        ImGui.EndMainMenuBar();
    }
}