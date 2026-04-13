#pragma once
#include <Rendering/DX12Includes.h>
#include <Rendering/Renderer.h>
#include <Rendering/Camera.h>

struct PushConstants;
class ConstantBuffer;
class LightBuffer;
class Texture;

class DX12Renderer : public Renderer
{
private:
	Camera m_Camera;

	struct IDXGIFactory2* m_DXGIFactory;
	struct ID3D12Device* m_Device;
	struct ID3D12InfoQueue* m_InfoQueue;
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
	struct ID3D12DescriptorHeap* m_MainSRVHeap;
	struct ID3D12DescriptorHeap* m_PerObjectSRVHeap;
	struct ID3D12DescriptorHeap* m_IMGUISRVHeap;
	int m_IMGUISRVIndex;
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
	HRESULT UpdateViewportAndScissorRect();

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_DefaultInputLayout;
	HRESULT CreateInputLayout();
	void DestroyInputLayout();

	char** m_ConstantBufferAddressArray;
	ID3D12Resource** m_ConstantBufferGPUUploaderArray;
	char** m_LightBufferAddressArray;
	ID3D12Resource** m_LightBufferGPUUploaderArray;
	HRESULT CreateConstantBuffers();
	void DestroyConstantBuffers();

	ID3D12DescriptorHeap** m_CBVHeaps;
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

	Matrix4x4 m_ProjectionMatrix;

	D3D12_CPU_DESCRIPTOR_HANDLE m_NullTextureDescriptor;
	HRESULT CreateNullDescriptors();
	void DestroyNullDescriptors();

	UINT m_IMGUISRVDescriptorHeapSize;

	void IMGUISRVAllocatorWithInfo(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle);
	void IMGUISRVDestructorWithInfo(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);
	void IMGUISRVAllocator(D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle);
	void IMGUISRVDestructor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

	PushConstants* m_PushConstants;
	void UploadPushConstants();

protected:

public:
	DX12Renderer();
	~DX12Renderer();

	ID3D12CommandQueue* GetCommandQueue();
	const ID3D12CommandQueue* GetCommandQueue() const;

	HRESULT ResetCommandList();
	ID3D12GraphicsCommandList* GetCommandList();
	const ID3D12GraphicsCommandList* GetCommandList() const;
	HRESULT ExecuteAndResetCommandList();

	const GraphicsDevice* GetDevice() const;
	GraphicsDevice* GetDevice();

	UINT GetSRVDescriptorHeapSize() const;
	struct D3D12_CPU_DESCRIPTOR_HANDLE GetMainSRVDescriptorHeapStartCPU() const;
	struct D3D12_GPU_DESCRIPTOR_HANDLE GetMainSRVDescriptorHeapStartGPU() const;
	struct D3D12_CPU_DESCRIPTOR_HANDLE GetDrawingSRVDescriptorHeapStartCPU() const;
	struct D3D12_GPU_DESCRIPTOR_HANDLE GetDrawingSRVDescriptorHeapStartGPU() const;

	HRESULT CreateDefaultBuffer(ID3D12Resource*& defaultBuffer, ID3D12Resource*& gpuUploadBuffer, const void* data, const size_t& sizeBytes);

	HRESULT UpdateWorldMatrix(const Matrix4x4& worldMatrix);
	HRESULT UpdateLightingBuffer(const LightBuffer& lb);
	HRESULT UpdateConstantBuffer(const ConstantBuffer& cb);

	bool Initialise(HWND windowHandle);
	bool InitialiseIMGUI();
	bool Shutdown();
	bool ShutdownIMGUI();

	const int& GetWindowWidth() const;
	const int& GetWindowHeight() const;

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetNullTextureDescriptor() const;
	HRESULT AssignTextureToSlot(const int& index, Texture* texture);

	const Matrix4x4& GetProjectionMatrix() const;
	Matrix4x4& GetProjectionMatrix();
	const Matrix4x4 GetViewMatrix() const;

	HRESULT ResizeSwapchain(const int& newWidth, const int& newHeight);

	void ClearFrame();
	HRESULT FlushCommandQueue();
	void PresentFrame();
};