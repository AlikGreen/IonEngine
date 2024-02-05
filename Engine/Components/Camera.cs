using System.Numerics;

namespace IonEngine;

public class Camera : Component
{

    public bool mainCamera = true;
    private Vector3 up = new Vector3(0.0f, 1.0f, 0.0f);

    public Matrix4x4 projection = Matrix4x4.Identity;
    public Matrix4x4 view = Matrix4x4.Identity;

    public Vector2 size = new Vector2(100);
    
    protected override void Start()
    {
        if(mainCamera)
            Renderer.camera = this;
    }
    
    protected override void Render()
    {
        view =  Matrix4x4.CreateLookAt(gameObject.transform.position, gameObject.transform.position + gameObject.transform.rotation, up);
        projection = Matrix4x4.CreatePerspectiveFieldOfView(Maths.DegreesToRadians(45.0f), size.X / size.Y, 0.1f, 1000.0f);
    }
}