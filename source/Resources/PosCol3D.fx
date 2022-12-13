
// -----------------------------------------------------------------
//  Global Variables
// -----------------------------------------------------------------

float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

// -----------------------------------------------------------------
//  Input/Ouput structs
// -----------------------------------------------------------------

struct VS_INPUT
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

// -----------------------------------------------------------------
//  Vertex shader
// -----------------------------------------------------------------

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.Position = mul(float4(input.Position, 1.0f), gWorldViewProj); // Multiply input position with the worldviewmatrix
    output.UV = input.UV;
    
    return output;
}

// -----------------------------------------------------------------
//  Pixel shader
// -----------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float3 color = gDiffuseMap.Sample(samPoint, input.UV).rgb;
    return float4(color, 1.0f);
}


// -----------------------------------------------------------------
//  Technique
// -----------------------------------------------------------------
technique11 DefaultTechnique
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
};
