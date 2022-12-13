#include "pch.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Camera.h"

namespace dae
{

	Renderer::Renderer(SDL_Window* pWindow):
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		// Init camera
		const float aspectRatio{ m_Width / (float)m_Height };
		m_pCamera = new Camera({ 0.f, 0.f, -10.0f }, 45.0f, aspectRatio);

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


		// Init mesh
		std::vector<Vertex> vertices{
			{{0.0f, 3.0f, 2.0f}, {1.f, 0.f, 0.f}},
			{{3.0f, -3.0f, 2.0f}, {0.f, 0.f, 1.f}},
			{{-3.0f, -3.0f, 2.0f}, {0.f, 1.f, 0.f}},
		};


		std::vector<uint32_t> indices{ 0, 1, 2 };

		m_pMesh = new Mesh{ m_pDevice, vertices, indices };
	}

	Renderer::~Renderer()
	{
		if(m_pDeviceContext)
		{
			m_pRenderTargetView->Release();
			m_pRenderTargetBuffer->Release();

			m_pDepthStencilView->Release();
			m_pDepthStencilBuffer->Release();

			m_pSwapChain->Release();

			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();

			m_pDevice->Release();

			delete m_pMesh;
		}

		delete m_pCamera;

	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_pCamera->Update(pTimer);
	}


	void Renderer::Render() const
	{
		if(!m_IsInitialized)
			return;

		//1. CLEAR RTV & DSV
		ColorRGB clearColor = ColorRGB{ 0.f, 0.f, 0.3f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		// 2. SET PIPELINE + INVOKE DRAWCALLS (= RENDER)
		Matrix worldViewProjectionMatrix{ m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix() };
		m_pMesh->Render(m_pDeviceContext, worldViewProjectionMatrix);

		// SWAP THE BACKBUFFER / PRESENT
		m_pSwapChain->Present(0, 0);
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
}
