float4 main(float2 pos: Position) : SV_POSTION
{
    return float4(pos.x, pos.y, 0.0f, 1.0f);
}   