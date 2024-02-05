using System.Numerics;
using IonEngine;
using Silk.NET.Input;

public class Magic : Component
{
    protected override void Update()
    {
        if(Input.GetKeyDown(MouseButton.Left))
        {
            GameObject ball = new GameObject("ball");
            ball.transform.position = gameObject.transform.position;
            ball.AddComponent(new MeshRenderer(Project.AssetsDirectory + "/Meshes/sphere.obj"));
            Rigidbody rb = new Rigidbody();
            rb.velocity.X = 4f;
            ball.AddComponent(rb);
            ball.transform.scale = new Vector3(0.2f);
            Window.scene.AddGameObject(ball);
        }
    }
}