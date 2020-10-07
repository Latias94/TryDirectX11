cbuffer CBuf
{
	float4 face_colors[6];
};

// 会告诉 pipeline 去为所有三角形生成一个 PrimitiveID
// pixel shader 会知道它在那个三角形中
float4 main(uint tid: SV_PrimitiveID) : SV_TARGET
{
	return face_colors[tid / 2]; // 每一面有两个三角形，这样就是一个面一个颜色
}