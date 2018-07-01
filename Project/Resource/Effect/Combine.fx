
matrix MatPalette[35];
int NumBoneInfluences = 2;

matrix Model;
matrix View;
matrix Projection;

static float3 LightDir = float3(1, 0, -1);
static vector LightColor = float4(1, 1, 1, 1);

float4 MorphWeights;

//Texture
texture TexDiffuse;
sampler DiffuseSampler = sampler_state
{
	Texture = (TexDiffuse);
	MinFilter = Linear;   MagFilter = Linear;   MipFilter = Linear;
	AddressU = Wrap;     AddressV = Wrap;     AddressW = Wrap;
	MaxAnisotropy = 16;
};

//================================Vertex Input================================//
//ÃÉÆ¤Íø¸ñ
struct VS_INPUT
{
	float4 Pos : POSITION0;
	float3 Nor   : NORMAL;
	float2 TexCoord     : TEXCOORD0;
	float4 Weights  : BLENDWEIGHT0;
	int4   BoneIndices : BLENDINDICES0;
};

//¾²Ì¬Íø¸ñ
struct VS_INPUT_STATIC
{
	float4 Pos : POSITION;
	float3 Nor   : NORMAL;
	float2 TexCoord : TEXCOORD0;
};

//Á³²¿±íÇé
struct VS_INPUT_MORPH
{
	float4 Pos0 : POSITION0;
	float3 Nor0   : NORMAL0;
	float2 TexCoord : TEXCOORD0;

	float4 Pos1 : POSITION1;
	float3 Nor1   : NORMAL1;

	float4 Pos2 : POSITION2;
	float3 Nor2  : NORMAL2;

	float4 Pos3 : POSITION3;
	float3 Nor3   : NORMAL3;

	float4 Pos4 : POSITION4;
	float3 Nor4   : NORMAL4;

	int4   BoneIndices : BLENDINDICES5;
	float4 Weights  : BLENDWEIGHT5;


};

//================================Vertex Output / Pixel Shader Input================================//
struct PS_INPUT
{
	float4 Pos : POSITION0;
	float2 TexCoord     : TEXCOORD0;
	float Factor : TEXCOORD1;
};

//================================Vertex Shader================================//
//ÃÉÆ¤Íø¸ñ
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
		pos += input.Weights[i] * mul( input.Pos, MatPalette[input.BoneIndices[i]] );
		norm += input.Weights[i] * ( mul( float4( input.Nor, 0.0 ), MatPalette[input.BoneIndices[i]] ).xyz );
	}
	lastWeight = 1.0f - lastWeight;

	pos += lastWeight * mul( input.Pos, MatPalette[input.BoneIndices[n]] );
	norm += lastWeight * ( mul( float4( input.Nor, 0.0 ), MatPalette[input.BoneIndices[n]] ).xyz );
	pos.w = 1.0f;

	//Transform vertex to world space
	float4 posWorld = mul(pos, Model);

	output.Pos = mul(mul(posWorld, View), Projection);

	output.TexCoord = input.TexCoord;

	float3 lightDir = normalize(LightDir);
	output.Factor = saturate(dot(norm, lightDir));

	return output;
}

//²»º¬ÃÉÆ¤ÐÅÏ¢µÄÍø¸ñ
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

//¾²Ì¬Íø¸ñ
PS_INPUT VS_Tex(VS_INPUT_STATIC input)
{
	PS_INPUT output = (PS_INPUT)0;

	input.Pos.w = 1.0;
	float4 posWorld = mul(input.Pos, Model);

	output.Pos = mul(mul(posWorld, View), Projection);

	float3 norm = normalize(input.Nor);
	norm = mul(float4(norm, 0.0), Model).xyz;

	float3 lightDir = normalize(LightDir);
	output.Factor = saturate(dot(norm, lightDir));

	output.TexCoord = input.TexCoord;

	return output;
}

//±íÇéÍø¸ñ
PS_INPUT VS_MorphSkin(VS_INPUT_MORPH input)
{
	PS_INPUT output = (PS_INPUT)0;

	//morph vetex
	float4 pos = input.Pos0 + (input.Pos1 - input.Pos0) * MorphWeights.x;
	pos += (input.Pos2 - input.Pos0) * MorphWeights.y;
	pos += (input.Pos3 - input.Pos0) * MorphWeights.z;
	pos +=  (input.Pos4 - input.Pos0) * MorphWeights.w;

	float3 norm = input.Nor0 + (input.Nor1 - input.Nor0) * MorphWeights.x;
	norm += (input.Nor2 - input.Nor0) * MorphWeights.y;
	norm += (input.Nor3 - input.Nor0) * MorphWeights.z;
	norm += (input.Nor4 - input.Nor0) * MorphWeights.w;

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

	//calculate world position and normal
	float4 posWorld = mul(finalPos, Model);
	output.Pos = mul(mul(posWorld, View), Projection);

	float3 norWorld = mul(finalNor, Model);
	float3 lightDir = normalize(LightDir);
	output.Factor = saturate(dot(norWorld, lightDir));

	output.TexCoord = input.TexCoord;

	return output;
}

//======================Pixel Shader========================//
float4 PS(PS_INPUT input) : COLOR0
{
	float4 color = tex2D(DiffuseSampler, input.TexCoord) * input.Factor * LightColor;
	return color;
}

//================================Technique================================//
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

technique TechTex
{
	pass P0
	{
		VertexShader = compile vs_2_0 VS_Tex();
		PixelShader = compile ps_2_0 PS();
	}
};

technique TechMorphSkin
{
	pass P0
	{
		VertexShader = compile vs_2_0 VS_MorphSkin();
		PixelShader = compile ps_2_0 PS();
	}
};