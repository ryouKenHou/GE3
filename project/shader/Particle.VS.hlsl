#include "Particle.hlsli"

StructuredBuffer<TransformationMatrix> gTransformationMatrix : register(t0);
StructuredBuffer<float32_t4> gParticleColor : register(t1);

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    TransformationMatrix transform = gTransformationMatrix[input.instanceId];

    output.position = mul(input.position, transform.WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) transform.World));
    output.color = gParticleColor[input.instanceId];

    return output;
}