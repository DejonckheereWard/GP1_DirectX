#pragma once
class Effect final
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);

	~Effect();
	Effect(const Effect&) = delete;
	Effect& operator=(const Effect&) = delete;
	Effect(Effect&&) = delete;
	Effect& operator=(Effect&&) = delete;

	ID3DX11Effect* GetEffect() const { return m_pEffect; };
	ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; };



private:
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;




};

