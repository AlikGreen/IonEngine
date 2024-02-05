using System.Numerics;

namespace IonEngine;

public class PointLight : Component
{
    public Vector3 color = new Vector3(1.0f);
    public float intensity = 1.0f;
    public float radius = 2.0f;
    public float fallOff = 0.5f;
    protected override void Create()
    {
        Renderer.pointLights.Add(this);
    }

    public void SetUniforms(ShaderProgram shader, int i)
    {
        shader.SetUniform($"pointLights[{i}].position", gameObject.transform.position);
        shader.SetUniform($"pointLights[{i}].color", color * intensity);
        shader.SetUniform($"pointLights[{i}].radius", radius);
        shader.SetUniform($"pointLights[{i}].fallOff", fallOff);
    }

    protected override void Delete()
    {
        Renderer.pointLights.Remove(this);
    }
}