module materialReflection;

import material;

type_param TMaterial : IMaterial;
ParameterBlock<TMaterial> material;

[shader("fragment")]
void fragmentMain()
{
    material.evaluate(SurfaceInput());
}

