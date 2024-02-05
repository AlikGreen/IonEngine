using Jitter;
using Jitter.Collision;

namespace IonEngine;

public class PhysicsWorld
{
    public World world = new World(new CollisionSystemBrute());
    public float timeScale = 1.0f;
    
    public void Update()
    {
        world.Step(Time.deltaTime * timeScale, true);
    }

    public void AddBody(Collider collider)
    {
        world.AddBody(collider.rb);
    }

    public void AddBody(Rigidbody rigidbody)
    {
        world.AddBody(rigidbody.rb);
    }
}