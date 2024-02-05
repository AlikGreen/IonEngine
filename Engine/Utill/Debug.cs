namespace IonEngine;

public static class Debug
{
    public static void Log(object log)
    {
        Console.ForegroundColor = ConsoleColor.White;
        Console.WriteLine("Info: " + log);
        Console.ForegroundColor = ConsoleColor.White;
    }  

    public static void Warn(object warning)
    {
        Console.ForegroundColor = ConsoleColor.Yellow;
        Console.WriteLine("Warning: " + warning);
        Console.ForegroundColor = ConsoleColor.White;
    }  

    public static void Error(object error)
    {
        Console.ForegroundColor = ConsoleColor.Red;
        Console.WriteLine("Error: " + error);
        Console.ForegroundColor = ConsoleColor.White;
    }  

    public static void Fatal(object fatal)
    {
        Console.ForegroundColor = ConsoleColor.Red;
        Console.WriteLine("\x1b[1mFatal: " + fatal + "\x1b[0m");
        Console.ForegroundColor = ConsoleColor.White;
    }  


    public static void DrawLine()
    {
        
    }
}