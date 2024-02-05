using IonEditor;

namespace IonEngine;

public static class Project
{
    public const string WorkspaceDirectory = "D:/Data/Projects/IonEngine/";
    public const string AssetsDirectory = "D:/Data/Projects/IonEngine/Assets/";
    public static void Launch(LaunchMode launchMode)
    {
        switch(launchMode)
        {
            case LaunchMode.Release:
                Window gameWindow = new GameWindow(1920, 1080, "Game");
                gameWindow.Run();
                break;
            case LaunchMode.Debug:
                Window debugWindow = new GameWindow(1920, 1080, "Game");
                debugWindow.Run();
                break;
            case LaunchMode.Editor:
                Window editorWindow = new EditorWindow(1920, 1080, "Game");
                editorWindow.Run();
                break;
        }
    }
}

public enum LaunchMode
{
    Release, Debug, Editor
}