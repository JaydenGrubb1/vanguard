static const int INDICES[6] = {
	0, 1, 2,
	0, 2, 3
};

static const float2 POSITIONS[4] = {
	float2(-1, -1),
	float2(-1,  1),
	float2( 1,  1),
	float2( 1, -1),
};

static const float2 UVS[4] = {
	float2(0, 1),
	float2(0, 0),
	float2(1, 0),
	float2(1, 1),
};

struct Varyings {
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Varyings VSmain(uint id : SV_VertexID) {
	Varyings output;
	
	int index = INDICES[id];
	output.position = float4(POSITIONS[index], 0.0, 1.0);
	output.uv = UVS[index];
	
	return output;
}

Texture2D t_texture : register(t0);
SamplerState s_sampler : register(s0);

float4 PSmain(Varyings input) : SV_TARGET {
	return t_texture.Sample(s_sampler, input.uv);
}
