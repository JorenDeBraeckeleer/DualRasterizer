//-----------------------------------------------//
// Global Variables
//-----------------------------------------------//
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorld : WORLD;
float4x4 gViewInverse : VIEWINVERSE;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

float3 gLightDirection = float3(0.577f, -0.577f, 0.577f);
float3 gLightColor = float3(1.0f, 1.0f, 1.0f);	
float3 gAmbientColor = float3(0.025f, 0.025f, 0.025f);

float gPi = 3.14159265358979323846264338327950288419716939937510f;
float gLightIntensity = 7.0f;
float gShininess = 25.0f;

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
	CullMode = back;
	FrontCounterClockwise = true;
};

RasterizerState gRasterizerStateBackCulling
{
	CullMode = back;
	FrontCounterClockwise = true;
};

RasterizerState gRasterizerStateFrontCulling
{
	CullMode = front;
	FrontCounterClockwise = true;
};

RasterizerState gRasterizerStateNoCulling
{
	CullMode = none;
	FrontCounterClockwise = true;
};

//-----------------------------------
// Blend States
//-----------------------------------
BlendState gBlendState
{
	BlendEnable[0] = false;
};

//-----------------------------------
// DepthStencil States
//-----------------------------------
DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = all;
	StencilEnable = true;
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
float4 ShadePixelLP(VS_OUTPUT input, SamplerState sampleState)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float3 viewDirection = normalize(output.WorldPosition.xyz - gViewInverse[3].xyz);
	
	//Normal
	float4 normalMapSample = gNormalMap.Sample(sampleState, input.TexCoord);
	normalMapSample = (normalMapSample * 2.0f) - float4(1.0f, 1.0f, 1.0f, 1.0f);
	float3 biNormal = cross(input.Normal, input.Tangent);
	float3x3 tangentSpaceAxis = float3x3(input.Tangent, biNormal, input.Normal);
	float3 newNormal = normalize(mul(normalMapSample.xyz, tangentSpaceAxis));
	
	//Diffuse
	float4 diffuseMapSample = gDiffuseMap.Sample(sampleState, input.TexCoord);
	float observedArea = max(dot(newNormal, -gLightDirection), 0.0f);
	float3 diffuseColor = gLightColor * (gLightIntensity / gPi) * diffuseMapSample.xyz * observedArea;
	
	//Specular + Glossiness
	float phongExponent = gGlossinessMap.Sample(sampleState, input.TexCoord).r * gShininess;
	float specularReflectance = gSpecularMap.Sample(sampleState, input.TexCoord).r;
	float3 reflect = normalize(gLightDirection - 2 * dot(newNormal, gLightDirection) * newNormal);
	float angle = saturate(dot(reflect, viewDirection));
	float phong = specularReflectance * pow(angle, phongExponent);
	float3 specularColor = float3(phong, phong, phong);
	
	float3 finalColor = gAmbientColor + diffuseColor + specularColor;
	return float4(finalColor, 1.0f);
}

float4 PSLPpoint(VS_OUTPUT input) : SV_TARGET
{
	return ShadePixelLP(input, gSamplePoint);
}

float4 PSLPlinear(VS_OUTPUT input) : SV_TARGET
{
	return ShadePixelLP(input, gSampleLinear);
}

float4 PSLPanisotropic(VS_OUTPUT input) : SV_TARGET
{
	return ShadePixelLP(input, gSampleAnisotropic);
}

float4 ShadePixelLP2(VS_OUTPUT input, SamplerState sampleState)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float3 viewDirection = normalize(output.WorldPosition.xyz - gViewInverse[3].xyz);
	
	//Normal
	float4 normalMapSample = gNormalMap.Sample(sampleState, input.TexCoord);
	normalMapSample = (normalMapSample * 2.0f) - float4(1.0f, 1.0f, 1.0f, 1.0f);
	float3 biNormal = cross(input.Normal, input.Tangent);
	float3x3 tangentSpaceAxis = float3x3(input.Tangent, biNormal, input.Normal);
	float3 newNormal = normalize(mul(normalMapSample.xyz, tangentSpaceAxis));

	//Specular + Glossiness
	float observedArea = max(dot(newNormal, -gLightDirection), 0.0f);
	float3 irradiance = gLightColor * gLightIntensity * observedArea;
	float4 specularColor = gSpecularMap.Sample(sampleState, input.TexCoord);
	float shininess = gGlossinessMap.Sample(sampleState, input.TexCoord).r * gShininess;
	float3 r = reflect(-gLightDirection, newNormal);
	float specularStrength = saturate(dot(r, viewDirection));
	float3 phongSpecularReflect = 1.0f * pow(specularStrength, shininess);
	float3 specular = specularColor.xyz * phongSpecularReflect;
	
	//Diffuse
	float4 diffuseMapSample = gDiffuseMap.Sample(sampleState, input.TexCoord);
	
	float3 finalColor = gAmbientColor + irradiance * (diffuseMapSample.xyz / gPi) + specular;
	return float4(finalColor, 1.0f);
}

float4 PSLP2point(VS_OUTPUT input) : SV_TARGET
{
	return ShadePixelLP2(input, gSamplePoint);
}

float4 PSLP2linear(VS_OUTPUT input) : SV_TARGET
{
	return ShadePixelLP2(input, gSampleLinear);
}

float4 PSLP2anisotropic(VS_OUTPUT input) : SV_TARGET
{
	return ShadePixelLP2(input, gSampleAnisotropic);
}

//-----------------------------------------------//
// Technique
//-----------------------------------------------//
technique11 LambertPhongTechnique
{
	pass Ppoint
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSLPpoint() ) );
	}
	
	pass Plinear
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSLPlinear() ) );
	}
	
	pass Panisotropic
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSLPanisotropic() ) );
	}
}

technique11 LambertPhongImprovedTechnique
{
	pass PpointB
	{
		SetRasterizerState(gRasterizerStateBackCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSLP2point() ) );
	}
	
	pass PpointF
	{
		SetRasterizerState(gRasterizerStateFrontCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSLP2point() ) );
	}
	
	pass PpointN
	{
		SetRasterizerState(gRasterizerStateNoCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSLP2point() ) );
	}
	
	pass PlinearB
	{
		SetRasterizerState(gRasterizerStateBackCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSLP2linear() ) );
	}
	
	pass PlinearF
	{
		SetRasterizerState(gRasterizerStateFrontCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSLP2linear() ) );
	}
	
	pass PlinearN
	{
		SetRasterizerState(gRasterizerStateNoCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSLP2linear() ) );
	}
	
	pass PanisotropicB
	{
		SetRasterizerState(gRasterizerStateBackCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSLP2anisotropic() ) );
	}
	
	pass PanisotropicF
	{
		SetRasterizerState(gRasterizerStateFrontCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSLP2anisotropic() ) );
	}
	
	pass PanisotropicN
	{
		SetRasterizerState(gRasterizerStateNoCulling);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSLP2anisotropic() ) );
	}
}