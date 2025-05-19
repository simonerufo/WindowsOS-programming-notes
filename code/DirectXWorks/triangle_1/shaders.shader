// Costante per le trasformazioni
cbuffer MatrixBuffer : register(b0)
{
    matrix modelMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VIn
{
    float4 position : POSITION;
    float4 color : COLOR;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VOut VShader(VIn input)
{
    VOut output;

    float4 modelPos = mul(input.position, modelMatrix);
    float4 viewPos = mul(modelPos, viewMatrix);
    output.position = mul(viewPos, projectionMatrix);

    output.color = input.color;
    return output;
}

float4 PShader(VOut input) : SV_TARGET
{
    return input.color;
}
