using System.Text.Json.Serialization;
using IonEditor;

namespace IonEngine;

public class MeshRenderer : Component
{
    public Mesh mesh;
    public Material material;

    [AcceptsFiles(".obj", ".fbx")]
    public string meshPath = "";
    [AcceptsFiles(".mat")]
    public string materialPath = "";

    [JsonIgnore]
    private string previousMat = "";
    [JsonIgnore]
    private string previousMesh = "";

    public MeshRenderer(string meshPath, string materialPath = Project.AssetsDirectory + "/Materials/default.mat")
    {
        this.meshPath = meshPath;
        this.materialPath = materialPath;
        mesh = new Mesh();
        material = new Material();

    }

    public MeshRenderer()
    {
        mesh = new Mesh();
        material = new Material();
    }


    protected override void Create()
    {
        Renderer.meshRenderers.Add(this);
    }

    protected override void Render()
    {
        if(previousMat != materialPath)
        {
            material = new Material();
            material.Load(materialPath);
            previousMat = materialPath;
        }
        if(previousMesh != meshPath)
        {
            mesh.Dispose();
            mesh = new Mesh();
            mesh.ParseFile(meshPath);
            mesh.GenerateBuffers();
            previousMesh = meshPath;
        }
    }

    protected override void Delete()
    {
        Renderer.meshRenderers.Remove(this);
        mesh = null;
    }
}