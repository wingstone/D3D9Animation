
matrix MatPalette[35];
int NumBoneInfluences = 2;

matrix Model;
matrix View;
matrix Projection;

float3 LightDir = float3(1, 0, -1);
vector LightColor = float4(1, 1, 1, 1);

//Texture
texture TexDiffuse;

//Sampler
sampler DiffuseSampler = sampler_state
{
	Texture = (TexDiffuse);
	MinFilter = Linear;   MagFilter = Linear;   MipFilter = Linear;
	AddressU = Wrap;     AddressV = Wrap;     AddressW = Wrap;
	MaxAnisotropy = 16;
};

//Vertex Input
struct VS_INPUT
{
	float4 Pos : POSITION0;
	float3 Nor   : NORMAL;
	float2 TexCoord     : TEXCOORD0;
	float4 Weights  : BLENDWEIGHT0;
	int4   BoneIndices : BLENDINDICES0;
};

//Vertex Output / Pixel Shader Input
struct PS_INPUT
{
	float4 Pos : POSITION0;
	float2 TexCoord     : TEXCOORD0;
	float Factor : TEXCOORD1;
};

PS_INPUT VS_Skin(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

	float4 pos = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float3 norm = float3(0.0f, 0.0f, 0.0f);
	float lastWeight = 0.0f;
	int n = NumBoneInfluences - 1;
	input.Nor = normalize(input.Nor);

	//Blend vertex position & normal
	for (int i = 0; i < n; ++i)
	{
		lastWeight += input.Weights[i];
		pos += input.Weights[i] * mul(input.Pos, MatPalette[input.BoneIndices[i]]);
		norm += input.Weights[i] * mul(input.Nor, MatPalette[input.BoneIndices[i]]);
	}
	lastWeight = 1.0f - lastWeight;

	pos += lastWeight * mul(input.Pos, MatPalette[input.BoneIndices[n]]);
	norm += lastWeight * mul(input.Nor, MatPalette[input.BoneIndices[n]]);
	pos.w = 1.0f;

	//Transform vertex to world space
	float4 posWorld = mul(pos, Model);

	//... then to screen space
	output.Pos = mul( mul(posWorld, View), Projection);

	//Copy UV coordinate
	output.TexCoord = input.TexCoord;

	float3 lightDir = normalize(LightDir);
	output.Factor = saturate(dot(norm, lightDir));

	return output;
}

PS_INPUT VS_NoSkin(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

	//Transform vertex to world space
	input.Pos.w = 1.0;
	float4 posWorld = mul(input.Pos, Model);

	//... then to screen space
	output.Pos = mul(mul(posWorld, View), Projection);

	//Copy UV coordinate
	output.TexCoord = input.TexCoord;

	float3 norm = normalize(input.Nor);
	norm = mul(float4(norm, 0.0), Model).xyz;

	float3 lightDir = normalize(LightDir);
	output.Factor = saturate(dot(norm, lightDir));

	return output;
}

//Pixel Shader
float4 PS(PS_INPUT input) : COLOR0
{
	float4 color = tex2D(DiffuseSampler, input.TexCoord) * input.Factor * LightColor;
	return color;
}

technique TechSkin
{
	pass P0
	{
		VertexShader = compile vs_2_0 VS_Skin();
		PixelShader = compile ps_2_0 PS();
	}
}

technique TechNoSkin
{
	pass P0
	{
		VertexShader = compile vs_2_0 VS_NoSkin();
		PixelShader = compile ps_2_0 PS();
	}
}