using System.Numerics;

namespace IonEngine;

public class DirectionalLight : Component
{
    public Vector3 color = new Vector3(1.0f);
    public float intensity = 1.0f;
    protected override void Create()
    {
        Renderer.directionalLights.Add(this);
    }

    public void SetUniforms(ShaderProgram shader, int i)
    {
        shader.SetUniform($"directionalLights[{i}].position", gameObject.transform.position);
        shader.SetUniform($"directionalLights[{i}].color", color * intensity);
    }

    protected override void Delete()
    {
        Renderer.directionalLights.Remove(this);
    }
}