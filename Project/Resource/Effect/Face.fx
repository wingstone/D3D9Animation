
float Weights[5];

matrix Model;
matrix View;
matrix Projection;

float3 LightDir = float3(1, 1, -1);
vector LightColor = float4(1, 1, 0.3, 1);

//Vertex Input
struct VS_INPUT
{
	float4 Pos0 : POSITION0;
	float3 Nor0   : NORMAL0;
	float2 TexCoord     : TEXCOORD0;

	float4 Pos1 : POSITION1;
	float3 Nor1   : NORMAL1;

	float4 Pos2 : POSITION2;
	float3 Nor2   : NORMAL2;

	float4 Pos3 : POSITION3;
	float3 Nor3   : NORMAL3;

	float4 Pos4 : POSITION4;
	float3 Nor4   : NORMAL4;
};

//Vertex Output / Pixel Shader Input
struct PS_INPUT
{
	float4 Pos : POSITION0;
	float2 TexCoord     : TEXCOORD0;
	float Factor : TEXCOORD1;
};

PS_INPUT VS_Face(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

	float4 pos = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float3 norm = float3(0.0f, 0.0f, 0.0f);

	pos += input.Pos0 * Weights[0];
	pos += input.Pos1 * Weights[1];
	pos += input.Pos2 * Weights[2];
	pos += input.Pos3 * Weights[3];
	pos += input.Pos4 * Weights[4];
	pos.w = 1.0;

	norm += input.Nor0 * Weights[0];
	norm += input.Nor1 * Weights[1];
	norm += input.Nor2 * Weights[2];
	norm += input.Nor3 * Weights[3];
	norm += input.Nor4 * Weights[4];

	//Transform vertex to world space
	float4 posWorld = mul(pos, Model);
	norm = mul(float4(norm, 0.0), Model).xyz;

	//... then to screen space
	output.Pos = mul(mul(posWorld, View), Projection);

	//Copy UV coordinate
	output.TexCoord = input.TexCoord;

	float3 lightDir = normalize(LightDir);
	output.Factor = saturate(dot(norm, lightDir));

	return output;
}

//Pixel Shader
float4 PS(PS_INPUT input) : COLOR0
{
	float4 color = input.Factor * LightColor;

	color = saturate(color);
	color.a = 1.0;
	return color;
}

technique TechFace
{
	pass P0
	{
		VertexShader = compile vs_2_0 VS_Face();
		PixelShader = compile ps_2_0 PS();
	}
}