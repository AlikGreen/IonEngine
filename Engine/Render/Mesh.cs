using Assimp;
using Newtonsoft.Json;
using Silk.NET.OpenGL;
using IonEditor;

namespace IonEngine;

[ShowInInspector]
public class Mesh
{
    public string filePath = "";

    [JsonIgnore]
    public uint vao;
    [JsonIgnore]
    private uint ebo, vbo, nbo, ubo;
    [JsonIgnore]
    public List<float> vertices = new List<float>();
    [JsonIgnore]
    private List<float> uvs = new List<float>();
    [JsonIgnore]
    private List<float> normals = new List<float>();
    [JsonIgnore]
    public List<uint> indices = new List<uint>();

    public void Clear()
    {
        vertices.Clear();
        uvs.Clear();
        normals.Clear();
        indices.Clear();
    }

    public bool ParseFile(string filePath)
    {
        try
        {
            AssimpContext importer = new AssimpContext();
            Assimp.Scene model = importer.ImportFile(filePath);
            List<int> indicesInts = new List<int>();

            foreach (Assimp.Mesh mesh in model.Meshes)
            {
                for (int i = 0; i < mesh.VertexCount; i++)
                {
                    vertices.Add(mesh.Vertices[i].X);
                    vertices.Add(mesh.Vertices[i].Y);
                    vertices.Add(mesh.Vertices[i].Z);

                    if (mesh.HasTextureCoords(0))
                    {
                        uvs.Add(mesh.TextureCoordinateChannels[0][i].X);
                        uvs.Add(mesh.TextureCoordinateChannels[0][i].Y);
                    }

                    if (mesh.HasNormals)
                    {
                        normals.Add(mesh.Normals[i].X);
                        normals.Add(mesh.Normals[i].Y);
                        normals.Add(mesh.Normals[i].Z);
                    }
                }

                indicesInts.AddRange(mesh.GetIndices());
            }

            indices.AddRange(indicesInts.ConvertAll(x => (uint)x));
            return true;
        }
        catch(Exception e)
        {
            Console.WriteLine("Couldent load. Error:");
            Console.WriteLine(e);
            return false;
        }
    }


    public unsafe void GenerateBuffers()
    {
        ebo = Window.gl.GenBuffer();
        vbo = Window.gl.GenBuffer();
        nbo = Window.gl.GenBuffer();
        ubo = Window.gl.GenBuffer();
        vao = Window.gl.GenVertexArray();

        Window.gl.BindVertexArray(vao);

        Window.gl.BindBuffer(BufferTargetARB.ArrayBuffer, vbo);
        fixed (float* ptr = vertices.ToArray())
        {        
            Window.gl.BufferData(GLEnum.ArrayBuffer, (nuint)vertices.Count * sizeof(float), ptr, GLEnum.DynamicDraw);
        }
        Window.gl.GetError();

        Window.gl.VertexAttribPointer(0, 3, GLEnum.Float, false, 3 * sizeof(float), (void*)0);
        Window.gl.EnableVertexAttribArray(0);

        Window.gl.BindBuffer(BufferTargetARB.ArrayBuffer, nbo);
        fixed (float* ptr = normals.ToArray())
        {
            Window.gl.BufferData(GLEnum.ArrayBuffer, (nuint)normals.Count * sizeof(float), ptr, GLEnum.DynamicDraw);
        }
        Window.gl.GetError();

        Window.gl.VertexAttribPointer(1, 3, GLEnum.Float, false, 3 * sizeof(float), (void*)0);
        Window.gl.EnableVertexAttribArray(1);

        Window.gl.BindBuffer(BufferTargetARB.ArrayBuffer, ubo);
        fixed (float* ptr = uvs.ToArray())
        {
            Window.gl.BufferData(GLEnum.ArrayBuffer, (nuint)(uvs.Count * sizeof(float)), ptr, GLEnum.DynamicDraw);
        }
        Window.gl.GetError();
        
        Window.gl.VertexAttribPointer(2, 2, GLEnum.Float, false, 2 * sizeof(float), (void*)0);
        Window.gl.EnableVertexAttribArray(2);

        Window.gl.BindBuffer(BufferTargetARB.ElementArrayBuffer, ebo);
        fixed (uint* ptr = indices.ToArray())
        {
            Window.gl.BufferData(GLEnum.ElementArrayBuffer, (nuint)indices.Count * sizeof(uint), ptr, GLEnum.StaticDraw);
        }
        Window.gl.GetError();

        Window.gl.BindBuffer(BufferTargetARB.ArrayBuffer, 0);
        Window.gl.BindVertexArray(0);
    }

    public unsafe void Draw()
    {
        Window.gl.BindVertexArray(vao);
        Window.gl.DrawElements(GLEnum.Triangles, (uint)indices.Count , DrawElementsType.UnsignedInt, (void*)0);
    }

    public void Dispose()
    {
        Window.gl.DeleteBuffer(vbo);
        Window.gl.DeleteBuffer(ubo);
        Window.gl.DeleteBuffer(nbo);
        Window.gl.DeleteBuffer(ebo);
        Window.gl.DeleteVertexArray(vao);
    }
}