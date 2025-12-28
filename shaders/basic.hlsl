cbuffer MVP : register(b0) {
	float4x4 mvp;
}

struct Attributes {
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct Varyings {
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Varyings VSmain(Attributes input) {
	Varyings output;
	
	output.position = mul(float4(input.position, 1.0f), mvp);
	output.uv = input.uv;
	
	return output;
}

Texture2D t_texture : register(t0);
SamplerState s_sampler : register(s0);

float4 PSmain(Varyings input) : SV_TARGET {
	return t_texture.Sample(s_sampler, input.uv);
}
