namespace IonEditor;

public class ShowInInspector : Attribute
{

}

public class HideInInspector : Attribute
{
    
}

public class AcceptsFiles : Attribute
{
    public string[] Extensions { get; }

    public AcceptsFiles(params string[] extensions)
    {
        Extensions = extensions;
    }
}

public class Range : Attribute
{
    
}