using System.Numerics;
using ImGuiNET;
using Silk.NET.Input;

namespace IonEngine;

public class EditorCamera : Component
{
    public float moveSpeed = 5f;
    float lookSensitivity = 10f;
    float pitch = 0;
    float yaw = 0;


    protected override void EditorUpdate()
    {
        Move();
    }

    private void Move()
    {
        Renderer.camera = gameObject.GetComponent<Camera>();
        float speed = Time.deltaTime * moveSpeed;
        float sensitivity = Time.deltaTime * lookSensitivity;
        
        if (!Input.GetKey(MouseButton.Right))
        {
            Input.SetCursorMode(CursorMode.Normal);
            return;
        }

        Input.SetCursorMode(CursorMode.Disabled);

        if (Input.GetKey(Key.W))
        {
            gameObject.transform.position += gameObject.transform.rotation * speed; //Forward 
        }

        if (Input.GetKey(Key.S))
        {
            gameObject.transform.position -= gameObject.transform.rotation * speed; //Backwards
        }

        if (Input.GetKey(Key.A))
        {
            gameObject.transform.position -= Vector3.Normalize(Vector3.Cross(gameObject.transform.rotation, Vector3.UnitY)) * speed; //Left
        }

        if (Input.GetKey(Key.D))
        {
            gameObject.transform.position += Vector3.Normalize(Vector3.Cross(gameObject.transform.rotation, Vector3.UnitY)) * speed; //Right
        }

        if (Input.GetKey(Key.Space))
        {
            gameObject.transform.position += Vector3.UnitY * speed; //Up 
        }

        if (Input.GetKey(Key.ShiftLeft))
        {
            gameObject.transform.position -= Vector3.UnitY * speed; //Down
        }


        yaw += Input.mouseDelta.X * sensitivity;
        pitch -= Input.mouseDelta.Y * sensitivity;

        pitch = Math.Clamp(pitch, -89f, 89f);

        gameObject.transform.rotation.X = (float)Math.Cos(Maths.DegreesToRadians(pitch)) * (float)Math.Cos(Maths.DegreesToRadians(yaw));
        gameObject.transform.rotation.Y = (float)Math.Sin(Maths.DegreesToRadians(pitch));
        gameObject.transform.rotation.Z = (float)Math.Cos(Maths.DegreesToRadians(pitch)) * (float)Math.Sin(Maths.DegreesToRadians(yaw));
        gameObject.transform.rotation = Vector3.Normalize(gameObject.transform.rotation);
    }
}