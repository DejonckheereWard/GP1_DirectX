#pragma once
#include "Matrix.h"

class Texture;

class Effect final
{
public:
	
	enum class SamplerFilter
	{
		Point,
		Linear,
		Anisotropic
	};

	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);

	~Effect();
	Effect(const Effect&) = delete;
	Effect& operator=(const Effect&) = delete;
	Effect(Effect&&) = delete;
	Effect& operator=(Effect&&) = delete;

	ID3DX11Effect* GetEffect() const { return m_pEffect; };
	ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; };
	ID3DX11EffectMatrixVariable* GetMatrixVariable() const { return m_pMatWorldViewProjVariable; };
	void SetDiffuseMap(Texture* pDiffuseTexture);
	//void CycleFilterMethod();

	void SetSamplerFilter(SamplerFilter filter);

private:
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;

	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	ID3DX11EffectSamplerVariable* m_pEffectSamplerVariable;

	// Store the different filter modes for the sampler
	ID3D11SamplerState* m_pPointSampler;
	ID3D11SamplerState* m_pLinearSampler;
	ID3D11SamplerState* m_pAnisotropicSampler;

	SamplerFilter m_CurrentSamplerFilter;

};

