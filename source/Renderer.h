#pragma once
#include "Effect.h"
#include "EffectVehicle.h"
#include "EffectFire.h"

using namespace dae;

struct SDL_Window;
struct SDL_Surface;
class Mesh;

class Camera;
class Texture;
class Renderer final
{
public:
	enum class FilterMethod
	{
		Point,
		Linear,
		Anisotropic
	};

	Renderer(SDL_Window* pWindow);
	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) noexcept = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) noexcept = delete;

	void Update(const Timer* pTimer);
	void Render() const;

	void CycleEffectFilter();

private:
	SDL_Window* m_pWindow{};

	int m_Width{};
	int m_Height{};

	bool m_IsInitialized{ false };

	//DIRECTX
	HRESULT InitializeDirectX();

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;

	IDXGISwapChain* m_pSwapChain;

	ID3D11Texture2D* m_pDepthStencilBuffer;
	ID3D11DepthStencilView* m_pDepthStencilView;

	ID3D11Texture2D* m_pRenderTargetBuffer;
	ID3D11RenderTargetView* m_pRenderTargetView;

	std::vector<Mesh*> m_MeshPtrs;

	Camera* m_pCamera;

	EffectVehicle* m_pVehicleMaterial;
	EffectFire* m_pFireMaterial;


	EffectVehicle::SamplerFilter m_FilterMethod;

};