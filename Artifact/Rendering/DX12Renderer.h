#pragma once
#include <Rendering/Renderer.h>
#include <Rendering/DX12Includes.h>

class ConstantBuffer;

class DX12Renderer : public Renderer
{
private:
	struct IDXGIFactory2* m_DXGIFactory;
	struct ID3D12Device* m_Device;
	HRESULT CreateDeviceAndFactory();
	void DestroyDeviceAndFactory();

	UINT m_RTVDescriptorHeapSize;
	UINT m_DSVDescriptorHeapSize;
	UINT m_CBVSRVDescriptorHeapSize;
	int m_CurrentFenceIndex;
	struct ID3D12Fence* m_Fence;
	HRESULT CreateFence();
	void DestroyFence();

	UINT m_m4xMSAAQuality;
	HRESULT DetermineMultisamplingDetails();
	void ClearMultisamplingDetails();

	struct ID3D12CommandQueue* m_CommandQueue;
	struct ID3D12CommandAllocator* m_CommandAllocator;
	struct ID3D12GraphicsCommandList* m_CommandList;
	HRESULT CreateCommandObjects();
	void DestroyCommandObjects();

	int m_CurrentBackbufferIndex;
	struct IDXGISwapChain1* m_SwapChain;
	HRESULT CreateSwapChain();
	void DestroySwapChain();

	struct ID3D12DescriptorHeap* m_RTVHeap;
	struct ID3D12DescriptorHeap* m_DSVHeap;
	HRESULT CreateDescriptorHeaps();
	void DestroyDescriptorHeaps();
	struct D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackbufferView() const;
	struct D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilBufferView() const;

	struct ID3D12Resource** m_SwapchainBuffers;
	HRESULT CreateRenderTargetViews();
	void DestroyRenderTargetViews();

	ID3D12Resource* m_DepthStencilBuffer;
	HRESULT CreateDepthStencilBuffer();
	void DestroyDepthStencilBuffer();

	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;
	HRESULT SetupInitialViewportAndScissorRect();

	HRESULT FlushCommandQueue();

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_DefaultInputLayout;
	HRESULT CreateInputLayout();
	void DestroyInputLayout();

	ID3D12Resource* m_ConstantBufferArray[MAX_NUM_ENTITIES];
	HRESULT CreateConstantBuffers();
	void DestroyConstantBuffers();

	ID3D12DescriptorHeap* m_CBVHeap;
	HRESULT CreateConstantBufferHeap();
	void DestroyConstantBufferHeap();

	ID3D12RootSignature* m_RootSignature;
	HRESULT CreateRootSignatureAndDescriptorTable();
	void DestroyRootSignatureAndDescriptorTable();

	ID3DBlob* m_DefaultPixelShaderBlob;
	ID3DBlob* m_DefaultVertexShaderBlob;
	HRESULT FindAndCreateShaders();
	HRESULT ReadShaderData(const std::string& filename, ID3DBlob*& targetBlob);
	void DestroyLoadedShaders();

	ID3D12PipelineState* m_DefaultPipeline;
	HRESULT CreateGraphicsPipelines();
	void DestroyGraphicsPipelines();

	bool InitialiseIMGUI();
	bool ShutdownIMGUI();

protected:

	bool Initialise(HWND windowHandle);
	bool Shutdown();

public:
	DX12Renderer();
	~DX12Renderer();

	HRESULT CreateResource(ID3D12Resource& resource, const D3D12_RESOURCE_DESC& resDesc);
	HRESULT UpdateConstantBuffer(const int& entityIndex, ConstantBuffer& cb);

	const GraphicsDevice* GetDevice() const;
	GraphicsDevice* GetDevice();

	void ClearFrame();
	void PresentFrame();
};