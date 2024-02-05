namespace IonEngine;

public class Movement : Component
{
    protected override void Update()
    {
        gameObject.transform.position.Z += Time.deltaTime;
    }
}