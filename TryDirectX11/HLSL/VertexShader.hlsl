struct VSOut
{
    float4 pos : SV_Position;
};

// 常数缓冲
cbuffer CBuf{
    matrix transform;
};

VSOut main(float3 pos : Position)
{
    VSOut vso;
    vso.pos = mul(float4(pos, 1.0f), transform);
    return vso;
}