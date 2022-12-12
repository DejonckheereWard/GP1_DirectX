#pragma once
#include "MathHelpers.h"
#include <vector>
using namespace dae;

class Effect;

struct Vertex
{
	Vector3 position{};
	ColorRGB color{};
};

struct Vertex_Out
{
	Vector4 position{};
	ColorRGB color{};
};


class Mesh final
{
public:
	Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices);

	~Mesh();
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator=(Mesh&&) = delete;

	void Render(ID3D11DeviceContext* pDeviceContext);

private:
	Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;

	ID3D11InputLayout* m_pInputLayout;

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;

	uint32_t m_NumIndices;


};
