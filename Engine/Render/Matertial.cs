using System.Numerics;
using Silk.NET.OpenGL;
using IonEditor;
using Newtonsoft.Json;

namespace IonEngine;

[ShowInInspector]
public class Material
{
    [JsonIgnore]
    public static List<ShaderProgram> shaders = new List<ShaderProgram>();
    public int shaderIndex = 0;
    
    public Texture albedoMap = null;
    public Texture metallicMap = null;
    public Texture roughnessMap = null;
    public Texture aoMap = null;
    public Vector3 albedo = new Vector3(1.0f);
    public float metallic;
    public float roughness;
    public float ao;

    FileSystemWatcher watcher = new FileSystemWatcher();

    public Material()
    {
        if(shaders.Count < 1)
        {
            Shader vert = new Shader(Project.AssetsDirectory + "Shaders/default.vert", ShaderType.VertexShader);
            Shader frag = new Shader(Project.AssetsDirectory + "Shaders/default.frag", ShaderType.FragmentShader);
            shaders.Add(new ShaderProgram(frag, vert));
        }

    }
    public void SetShaderProperties()
    {
        shaders[shaderIndex].Use();

        shaders[shaderIndex].SetUniform("mat.albedo", albedo);
        shaders[shaderIndex].SetUniform("mat.metallic", metallic);
        shaders[shaderIndex].SetUniform("mat.roughness", roughness);
        shaders[shaderIndex].SetUniform("mat.ao", ao);

        if(albedoMap != null)
        {
            shaders[shaderIndex].SetUniform("mat.useAlbedoMap", true);
            albedoMap.Bind(TextureUnit.Texture0);
        }
        else
        {
            shaders[shaderIndex].SetUniform("mat.useAlbedoMap", false);
        }

        if(metallicMap != null)
        {
            shaders[shaderIndex].SetUniform("mat.useMetallicMap", true);
            metallicMap.Bind(TextureUnit.Texture1);
        }
        else
        {
            shaders[shaderIndex].SetUniform("mat.useMetallicMap", false);
        }

        if(roughnessMap != null)
        {
            shaders[shaderIndex].SetUniform("mat.useRoughnessMap", true);
            roughnessMap.Bind(TextureUnit.Texture2);
        }
        else
        {
            shaders[shaderIndex].SetUniform("mat.useRoughnessMap", false);
        }

        if(aoMap != null)
        {
            shaders[shaderIndex].SetUniform("mat.useAoMap", true);
            aoMap.Bind(TextureUnit.Texture3);
        }
        else
        {
            shaders[shaderIndex].SetUniform("mat.useAoMap", false);
        }
    }

    public void Load(string path)
    {
        if(File.Exists(path))
        {
            string directoryPath = Path.GetDirectoryName(path);
            string fileName = Path.GetFileName(path);

            watcher.Path = directoryPath;
            watcher.Filter = fileName;
            watcher.NotifyFilter = NotifyFilters.LastWrite;
            watcher.Changed += (source, e) => Load(path);
            watcher.EnableRaisingEvents = true;
        }
        try
        {
            string text = File.ReadAllText(path);
            JsonSerializerSettings settings = new JsonSerializerSettings
            {
                PreserveReferencesHandling = PreserveReferencesHandling.All,
                TypeNameHandling = TypeNameHandling.All
            };
            JsonConvert.PopulateObject(text, this, settings);
        }
        catch(Exception e)
        {
            Console.WriteLine("Failed to load material. Error:");
            Console.WriteLine(e);
        }
    }

    public void Save(string path)
    {
        try
        {
            JsonSerializerSettings settings = new JsonSerializerSettings
            {
                PreserveReferencesHandling = PreserveReferencesHandling.All,
                TypeNameHandling = TypeNameHandling.All
            };
            string fileText = JsonConvert.SerializeObject(this, settings);
            File.WriteAllText(path, fileText);
        }
        catch(Exception e)
        {
            Console.WriteLine("Failed to save material. Error:");
            Console.WriteLine(e);
        }
    }


    public void Dispose()
    {
        albedoMap?.Dispose();
        metallicMap?.Dispose();
        roughnessMap?.Dispose();
        aoMap?.Dispose();
    }
}
