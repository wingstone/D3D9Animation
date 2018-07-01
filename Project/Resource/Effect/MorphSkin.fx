matrix MatPalette[35];
int NumBoneInfluences = 2;

float MorphWeight = 0;

matrix Model;
matrix View;
matrix Projection;

float3 LightDir = float3(1, 0, -1);
vector LightColor = float4(1, 1, 1, 1);

//Texture
texture TexHuman;
texture TexWolf;

//Sampler
sampler HumanSampler = sampler_state
{
	Texture = (TexHuman);
	MinFilter = Linear;   MagFilter = Linear;   MipFilter = Linear;
	AddressU = Wrap;     AddressV = Wrap;     AddressW = Wrap;
	MaxAnisotropy = 16;
};
sampler WolfSampler = sampler_state
{
	Texture = (TexWolf);
	MinFilter = Linear;   MagFilter = Linear;   MipFilter = Linear;
	AddressU = Wrap;     AddressV = Wrap;     AddressW = Wrap;
	MaxAnisotropy = 16;
};

//Vertex Input
struct VS_INPUT
{
	float4 Pos0 : POSITION0;
	float3 Nor0   : NORMAL0;

	float2 TexCoord     : TEXCOORD0;
	int4   BoneIndices : BLENDINDICES0;
	float4 Weights  : BLENDWEIGHT0;

	float4 Pos1 : POSITION1;
	float3 Nor1   : NORMAL1;
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

	float4 pos = input.Pos0 + (input.Pos1 - input.Pos0) * MorphWeight;
	float3 norm = input.Nor0 + (input.Nor1 - input.Nor0) * MorphWeight;

	float lastWeight = 0.0f;
	int n = NumBoneInfluences - 1;

	float4 finalPos = float4(0, 0, 0, 0);
	float3 finalNor = float3(0, 0, 0);

	//Blend vertex position & normal
	for (int i = 0; i < n; ++i)
	{
		lastWeight += input.Weights[i];
		finalPos += input.Weights[i] * mul(pos, MatPalette[input.BoneIndices[i]]);
		finalNor += input.Weights[i] * mul(float4(norm, 0.0), MatPalette[input.BoneIndices[i]]).xyz;
	}
	lastWeight = 1.0f - lastWeight;

	finalPos += lastWeight * mul(pos, MatPalette[input.BoneIndices[n]]);
	finalNor += lastWeight * mul(float4(norm, 0.0), MatPalette[input.BoneIndices[n]]).xyz;
	finalPos.w = 1.0f;

	//Transform vertex to world space
	float4 posWorld = mul(finalPos, Model);

	//... then to screen space
	output.Pos = mul(mul(posWorld, View), Projection);

	//Copy UV coordinate
	output.TexCoord = input.TexCoord;

	float3 lightDir = normalize(LightDir);
	output.Factor = saturate(dot(finalNor, lightDir));

	return output;
}

//Pixel Shader
float4 PS(PS_INPUT input) : COLOR0
{
	float4 color1 = tex2D(HumanSampler, input.TexCoord);
	float4 color2 = tex2D(WolfSampler, input.TexCoord);

	float4 color = lerp(color1, color2, MorphWeight) * input.Factor * LightColor;

	color = saturate(color);
	color.a = 1.0;
	return color;
}

technique TechMorphSkin
{
	pass P0
	{
		VertexShader = compile vs_2_0 VS_Skin();
		PixelShader = compile ps_2_0 PS();
	}
}