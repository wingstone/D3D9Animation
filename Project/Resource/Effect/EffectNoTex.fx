matrix Model;
matrix View;
matrix Projection;

struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Nor   : NORMAL;
	float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : POSITION;
	float3 Nor   : TEXCOORD0;
	float2 Tex : TEXCOORD1;
};

PS_INPUT VS_main(VS_INPUT input)
{
	PS_INPUT output;
	float4 pos = float4(input.Pos.xyz, 1.0f);
	float4 worldPos = mul(pos, Model);
	output.Pos = mul(mul(worldPos, View), Projection);

	output.Nor = mul(float4(input.Nor, 0.0), Model).xyz;
	output.Nor = normalize(output.Nor);

	output.Tex = input.Tex;

	return output;
}

float4 PS_main(PS_INPUT input) : COLOR
{
	float3 lightDir = normalize(float3(1.0, 1.0, -1.0));
	float factor = saturate(dot(input.Nor, lightDir));

	float4 lightCol = float4(1.0, 1.0, 0.3, 1.0);
	float4 color = lightCol * factor;

	color = saturate(color);
	color.a = 1.0;
	return color;
}


technique Tech
{
	pass p0
	{
		VertexShader = compile vs_2_0 VS_main();
		PixelShader = compile ps_2_0 PS_main();
		//fvf = XYZ | TEX1;
	}
}