#include "pch.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Camera.h"
#include "Texture.h"
#include "Utils.h"


Renderer::Renderer(SDL_Window* pWindow):
	m_pWindow(pWindow),
	m_FilterMethod{ }
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	// Init camera
	const float aspectRatio{ m_Width / (float)m_Height };
	m_pCamera = new Camera({ 0.f, 0.f, -50.0f }, 45.0f, aspectRatio);

	//Initialize DirectX pipeline
	const HRESULT result = InitializeDirectX();
	if(result == S_OK)
	{
		m_IsInitialized = true;
		std::cout << "DirectX is initialized and ready!\n";
	}
	else
	{
		std::cout << "DirectX initialization failed!\n";
	}


	m_pVehicleMaterial = new EffectVehicle{ m_pDevice, L"Resources/PosCol3D.fx" };

	Texture* pVehicleDiffuse = Texture::LoadFromFile(m_pDevice, "./Resources/vehicle_diffuse.png");
	Texture* pVehicleNormal = Texture::LoadFromFile(m_pDevice, "./Resources/vehicle_normal.png");
	Texture* pVehicleSpecular = Texture::LoadFromFile(m_pDevice, "./Resources/vehicle_specular.png");
	Texture* pVehicleGloss = Texture::LoadFromFile(m_pDevice, "./Resources/vehicle_gloss.png");

	m_pVehicleMaterial->SetDiffuseMap(pVehicleDiffuse);
	m_pVehicleMaterial->SetNormalMap(pVehicleNormal);
	m_pVehicleMaterial->SetSpecularMap(pVehicleSpecular);
	m_pVehicleMaterial->SetGlossinessMap(pVehicleGloss);

	delete pVehicleDiffuse;
	delete pVehicleNormal;
	delete pVehicleSpecular;
	delete pVehicleGloss;

	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};

	Utils::ParseOBJ("./Resources/vehicle.obj", vertices, indices);
	Mesh* pMesh = m_MeshPtrs.emplace_back(new Mesh{ m_pDevice, m_pVehicleMaterial, vertices, indices });

	m_pFireMaterial = new EffectFire{ m_pDevice, L"Resources/ShaderTransparent.fx" };
	Texture* pFireDiffuse = Texture::LoadFromFile(m_pDevice, "./Resources/fireFX_diffuse.png");
	m_pFireMaterial->SetDiffuseMap(pFireDiffuse);
	delete pFireDiffuse;


	Utils::ParseOBJ("./Resources/fireFX.obj", vertices, indices);
	pMesh = m_MeshPtrs.emplace_back(new Mesh{ m_pDevice, m_pFireMaterial, vertices, indices });

}

Renderer::~Renderer()
{
	if(m_pDeviceContext)
	{
		delete m_pVehicleMaterial;
		delete m_pFireMaterial;

		SafeRelease(m_pRenderTargetView);
		SafeRelease(m_pRenderTargetBuffer);

		SafeRelease(m_pDepthStencilView);
		SafeRelease(m_pDepthStencilBuffer);

		SafeRelease(m_pSwapChain);


		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();

		SafeRelease(m_pDevice);

		for(Mesh* pMesh : m_MeshPtrs)
		{
			delete pMesh;
		}
	}

	delete m_pCamera;
}

void Renderer::Update(const Timer* pTimer)
{
	m_pCamera->Update(pTimer);

	Matrix rotation = Matrix::CreateRotationY(PI_DIV_4 * pTimer->GetElapsed());

	for(Mesh* pMesh : m_MeshPtrs)
	{
		pMesh->SetWorldMatrix(pMesh->GetWorldMatrix() * rotation);
	}

}


void Renderer::Render() const
{
	if(!m_IsInitialized)
		return;

	//1. CLEAR RTV & DSV
	ColorRGB clearColor = ColorRGB{ 0.3f, 0.3f, 0.3f };
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	// 2. SET PIPELINE + INVOKE DRAWCALLS (= RENDER)
	for(Mesh* pMesh : m_MeshPtrs)
	{
		Matrix worldViewProjectionMatrix{ pMesh->GetWorldMatrix() * m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix() };
		pMesh->Render(m_pDeviceContext, worldViewProjectionMatrix, m_pCamera->GetInverseViewMatrix());
	}

	// SWAP THE BACKBUFFER / PRESENT
	m_pSwapChain->Present(0, 0);
}

void Renderer::CycleEffectFilter()
{
	// Fancy thing to toggle / go through the filtermethods 1 by 1
	m_FilterMethod = EffectVehicle::SamplerFilter((int(m_FilterMethod) + 1) % (int(EffectVehicle::SamplerFilter::Anisotropic) + 1));

	switch(m_FilterMethod)
	{
		case EffectVehicle::SamplerFilter::Point:
			std::wcout << "FilterMethod: Point\n";
			break;
		case EffectVehicle::SamplerFilter::Linear:
			std::wcout << "FilterMethod: Linear\n";
			break;
		case EffectVehicle::SamplerFilter::Anisotropic:
			std::wcout << "FilterMethod: Anisotropic\n";
			break;
		default:
			std::wcout << "FilterMethod: ERROR - INVALID\n";
			break;
	}

	// loop over every mesh and set the effect
	for(Mesh* pMesh : m_MeshPtrs)
	{
		pMesh->GetEffect()->SetSamplerFilter(m_FilterMethod);
	}


}

HRESULT Renderer::InitializeDirectX()
{
	// 1. Create device & device context
	// ==============================

	D3D_FEATURE_LEVEL featureLevel{ D3D_FEATURE_LEVEL_11_1 };
	uint32_t createDeviceFlags{ 0 };

#if defined(debug) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result{ D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		createDeviceFlags,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&m_pDevice,
		nullptr,
		&m_pDeviceContext
		) };

	if(FAILED(result))
	{
		std::cout << "Error creating device\n";
		return result;
	}


	// Create DXGI Factory
	IDXGIFactory1* pDxgiFactory{};
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));

	if(FAILED(result))
	{
		std::cout << "Error creating factory\n";
		return result;
	}


	// 2. Create Swapchain
	// ==============================

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;


	// Get the handle (HWND) from the SDL backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	// Create the swapchain using the given settings
	result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	pDxgiFactory->Release();

	if(FAILED(result))
	{
		std::cout << "Error getting SDL window\n";
		return result;
	}


	// 3. Create the DepthStencis (DS) & DepthStencilView (DSV)
	// ==============================
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	// View
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
	if(FAILED(result))
	{
		std::cout << "Error creating depth stencil\n";
		return result;
	}

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if(FAILED(result))
	{
		std::cout << "Error creating depth stencil view\n";
		return result;
	}

	// 4. Create RenderTraget (RT) & RenderTargetView (RTV)
	// ==============================

	// Resource
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if(FAILED(result))
	{
		std::cout << "Error creating render target buffer\n";
		return result;
	}

	// View
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
	if(FAILED(result))
	{
		std::cout << "Error creating render target view\n";
		return result;
	}

	// 5. Bind RTV & DSV to Output Merger Stage
	// ==============================
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// 6. Set the Viewport
	// ==============================
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(m_Width);
	viewPort.Height = static_cast<float>(m_Height);
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);



	return result;
}
