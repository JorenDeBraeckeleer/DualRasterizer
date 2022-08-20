//-----------------------------------------------//
// Global Variables
//-----------------------------------------------//
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorld : WORLD;

Texture2D gDiffuseMap : DiffuseMap;

//-----------------------------------------------//
// Sampler States
//-----------------------------------------------//
SamplerState gSamplePoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Border;
	AddressV = Clamp;
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};

SamplerState gSampleLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Border;
	AddressV = Clamp;
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};

SamplerState gSampleAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Border;
	AddressV = Clamp;
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};

//-----------------------------------
// Rasterizer States
//-----------------------------------
RasterizerState gRasterizerState
{
	CullMode = none;
	FrontCounterClockwise = true;
};

//-----------------------------------
// Blend States
//-----------------------------------
BlendState gBlendState
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

//-----------------------------------
// DepthStencil States
//-----------------------------------
DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less;
	StencilEnable = false;
	
	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0F;
	
	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;
	
	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;
	
	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;
	
	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};

//-----------------------------------------------//
// Input/Output Structs
//-----------------------------------------------//
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Color : COLOR;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};
struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : COLOR;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

//-----------------------------------------------//
// Vertex Shader
//-----------------------------------------------//
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.0f), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.0f), gWorld);
	output.TexCoord = input.TexCoord;
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorld);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorld);
	
	return output;
}

//-----------------------------------------------//
// Pixel Shader
//-----------------------------------------------//
float4 ShadePixel(VS_OUTPUT input, SamplerState sampleState)
{
	return gDiffuseMap.Sample(sampleState, input.TexCoord);
}

float4 PSpoint(VS_OUTPUT input) : SV_TARGET
{
	return ShadePixel(input, gSamplePoint);
}

float4 PSlinear(VS_OUTPUT input) : SV_TARGET
{
	return ShadePixel(input, gSampleLinear);
}

float4 PSanisotropic(VS_OUTPUT input) : SV_TARGET
{
	return ShadePixel(input, gSampleAnisotropic);
}

//-----------------------------------------------//
// Technique
//-----------------------------------------------//
technique11 FilterTechnique
{
	pass Ppoint
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSpoint() ) );
	}
	
	pass Plinear
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSlinear() ) );
	}
	
	pass Panisotropic
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSanisotropic() ) );
	}
}