cbuffer PushConstants : register(b0) {
	float4x4 model;
	float4 tint;
}

cbuffer UniformBuffer : register(b1) {
	float4x4 view;
	float4x4 proj;
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
	
	float4 pos = float4(input.position, 1.0);
	
	pos = mul(model, pos);
	pos = mul(view, pos);
	pos = mul(proj, pos);
	
	output.position = pos;
	output.uv = input.uv;
	
	return output;
}

Texture2D t_texture : register(t0);
SamplerState s_sampler : register(s0);

float4 PSmain(Varyings input) : SV_TARGET {
	float4 sample = t_texture.Sample(s_sampler, input.uv);
	return sample * tint;
}
