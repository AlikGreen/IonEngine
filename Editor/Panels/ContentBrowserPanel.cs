using System.Numerics;
using System.Text;
using ImGuiNET;
using IonEngine;

namespace IonEditor;

public class ContentBrowserPanel
{
    private string currentDirectory = Project.AssetsDirectory;
    private float iconScale = 1f;
    private float timeSinceLastClick = 100f;


    private float timeForDoubleClick = 0.25f;

    private Texture folderThumbnail;
    private Texture fileThumbnail;

    public ContentBrowserPanel()
    {
        folderThumbnail = new Texture(Project.WorkspaceDirectory + "Assets/Thumbnails/folder.png");
        fileThumbnail = new Texture(Project.WorkspaceDirectory + "Assets/Thumbnails/document.png");
    }

    public unsafe string Draw()
    {
        Random random = new Random();
        timeSinceLastClick += Time.deltaTime;

        string[] files = Directory.GetFiles(currentDirectory);
        string[] directories = Directory.GetDirectories(currentDirectory);

        ImGui.Begin("Content Browser", ImGuiWindowFlags.NoCollapse);

        if (ImGui.IsWindowHovered() && ImGui.IsMouseReleased(ImGuiMouseButton.Right))
        {
            ImGui.OpenPopup("Create");
        }

        if (ImGui.BeginPopup("Create"))
        {
            if (ImGui.Selectable("Create Material"))
            {
                Material material = new Material();
                material.Save(currentDirectory+"/material" + random.NextSingle() + ".mat");
            }

            ImGui.EndPopup();
        }

        string fullCurrentDirectory = Path.GetFullPath(currentDirectory);
        string fullWorkspaceDirectory = Path.GetFullPath(Project.AssetsDirectory);

        fullCurrentDirectory = Path.TrimEndingDirectorySeparator(fullCurrentDirectory);
        fullWorkspaceDirectory = Path.TrimEndingDirectorySeparator(fullWorkspaceDirectory);

        if (!fullCurrentDirectory.Equals(fullWorkspaceDirectory, StringComparison.OrdinalIgnoreCase) && ImGui.Button("<"))
        {
            try
            {
                currentDirectory = Directory.GetParent(currentDirectory).FullName;
            }catch
            {

            }
        }

        ImGui.SameLine();
        ImGui.Text(currentDirectory);
        ImGui.SameLine();
        ImGui.Spacing();
        ImGui.SameLine();
        ImGui.SliderFloat("Icon Scale", ref iconScale, 0.25f, 3f);

        ImGui.PushStyleColor(ImGuiCol.Button, new Vector4(0, 0, 0, 0));
        ImGui.PushStyleColor(ImGuiCol.ButtonHovered, new Vector4(0.4f, 0.4f, 0.4f, 0.8f));

        
        float windowWidth = ImGui.GetContentRegionAvail().X;
        int buttonsPerRow = 8;
        float buttonWidth = Math.Clamp(windowWidth/buttonsPerRow, 48f, 128f) * iconScale; 
        float buttonPadding = 20;
        float buttonsFitPerRow = windowWidth / (buttonWidth + buttonPadding);
        
        int buttonCount = 0;
        foreach (string directory in directories)
        {
            ImGui.BeginGroup();
            if (ImGui.ImageButton(Path.GetFileName(directory), (IntPtr)folderThumbnail.id, new Vector2(buttonWidth), new Vector2(0, 1), new Vector2(1, 0)))
            {
                currentDirectory = directory;
            }
            string truncatedText = CalcTruncatedTextWidth(Path.GetFileName(directory), buttonWidth);
            ImGui.Text(truncatedText);
            ImGui.EndGroup(); 
        
            buttonCount++;
            if ((buttonCount + 1) < buttonsFitPerRow)
            {
                ImGui.SameLine();
            }else
            {
                buttonCount = 0;
            }

        }
        
        foreach (string file in files)
        {
            ImGui.BeginGroup();
            if(ImGui.ImageButton(Path.GetFileName(file), (IntPtr)fileThumbnail.id, new Vector2(buttonWidth), new Vector2(0, 1), new Vector2(1, 0)))
            {
                if(timeSinceLastClick < timeForDoubleClick)
                {
                    ImGui.EndGroup();
                    ImGui.End();
                    return file;
                }
                timeSinceLastClick = 0f;
            }


            if (ImGui.BeginDragDropSource())
            {
                byte[] payloadData = Encoding.UTF8.GetBytes(file);
                fixed (byte* ptr = payloadData)
                {
                    ImGui.SetDragDropPayload("CONTENT_BROWSER_ITEM", (nint)ptr, (uint)payloadData.Length);
                }
                ImGui.EndDragDropSource();
            }
            string truncatedText = CalcTruncatedTextWidth(Path.GetFileName(file), buttonWidth);
            ImGui.Text(truncatedText); 
            ImGui.EndGroup();
        
            buttonCount++;

            if ((buttonCount + 1) < buttonsFitPerRow)
            {
                ImGui.SameLine();
            }else
            {
                buttonCount = 0;
            }

        }
        
        ImGui.PopStyleColor(2);
        
        ImGui.End();

        return null;
    }

    string CalcTruncatedTextWidth(string text, float maxWidth)
    {
        string truncatedText = text;
        Vector2 textSize = ImGui.CalcTextSize(truncatedText);

        while (textSize.X > maxWidth && truncatedText.Length > 0)
        {
            if(truncatedText.Length <= 4) break;
            
            truncatedText = truncatedText.Substring(0, truncatedText.Length - 4) + "...";
            textSize = ImGui.CalcTextSize(truncatedText);

        }

        return truncatedText;
    }
}