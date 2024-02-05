using System.Numerics;
using Jitter;
using Jitter.Collision;
using Jitter.Collision.Shapes;
using Jitter.Dynamics;
using Jitter.LinearMath;

namespace IonEngine;

public unsafe class Rigidbody : Component
{
    public Vector3 velocity = new Vector3();
    private static World world;

    public RigidBody rb;

    protected override void Start()
    {
        if(world == null)
        {
            world = new World(new CollisionSystemBrute());
        }

        rb = new RigidBody(new BoxShape(gameObject.transform.scale.X*2, gameObject.transform.scale.Y*2, gameObject.transform.scale.X*2))
        {
            Position = new JVector(gameObject.transform.position.X, gameObject.transform.position.Y, gameObject.transform.position.Z),
            IsStatic = false
        };
        Window.physicsWorld.AddBody(this);
    }
    protected override void Update()
    {
        rb.LinearVelocity = new JVector(velocity.X, velocity.Y, velocity.Z);
        rb.Position = new JVector(gameObject.transform.position.X, gameObject.transform.position.Y, gameObject.transform.position.Z);

        rb.Orientation = JMatrix.CreateFromYawPitchRoll(gameObject.transform.rotation.X, gameObject.transform.rotation.Y, gameObject.transform.rotation.Z);
    }
    protected override void LateUpdate()
    {
        velocity = new Vector3(rb.LinearVelocity.X, rb.LinearVelocity.Y, rb.LinearVelocity.Z);
        gameObject.transform.position = new Vector3(rb.Position.X, rb.Position.Y, rb.Position.Z);
        gameObject.transform.rotation = Maths.MatrixToEuler(rb.Orientation);
    }
}

