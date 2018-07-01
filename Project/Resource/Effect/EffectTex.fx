matrix Model;
matrix View;
matrix Projection;

texture TexDiffuse;

sampler DiffuseSampler = sampler_state
{
	Texture = (TexDiffuse);
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
};

struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

PS_INPUT VS_main(VS_INPUT input)
{
	PS_INPUT output;
	float4 pos = float4(input.Pos.xyz, 1.0f);
	float4 worldPos = mul(pos, Model);
	output.Pos = mul( mul( worldPos, View ), Projection );
	
	output.Tex = input.Tex;
	
	return output;
}

float4 PS_main(PS_INPUT input) : COLOR
{
	float4 color = tex2D(DiffuseSampler, input.Tex);
	
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