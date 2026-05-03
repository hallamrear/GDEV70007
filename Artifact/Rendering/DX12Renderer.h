#pragma once
#include <Rendering/IMGUIRenderable.h>
#include <Rendering/DX12Includes.h>
#include <Rendering/Renderer.h>

struct PushConstants;
class ConstantBuffer;
class LightBuffer;
class Texture;
class GenericBuffer;
class VertexBuffer;
class IndexBuffer;
class Mesh;

class DX12Renderer : public Renderer, IIMGUIRenderable
{
private:
	struct IDXGIFactory2* m_DXGIFactory;
	static struct ID3D12Device* m_Device;
	struct ID3D12InfoQueue* m_InfoQueue;
	HRESULT CreateDeviceAndFactory();
	void DestroyDeviceAndFactory();

	static UINT m_RTVDescriptorHeapSize;
	static UINT m_DSVDescriptorHeapSize;
	static UINT m_CBVSRVDescriptorHeapSize;
	int m_CurrentFenceIndex;
	struct ID3D12Fence* m_Fence;
	HRESULT CreateFence();
	void DestroyFence();

	bool m_UseMultisampling;
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
	static struct ID3D12DescriptorHeap* m_MainStorageSRVHeap;
	static struct ID3D12DescriptorHeap* m_DrawCopySRVHeap;
	static struct ID3D12DescriptorHeap* m_IMGUISRVHeap;
	static int m_MainStorageSRVHeapDescriptorEndIndex;
	static int m_DrawSRVHeapDescriptorEndIndex;
	static int m_IMGUISRVHeapDescriptorEndIndex;
	static void GetNewDescriptorHandleFromMainSRVHeap(D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle);
	static void IMGUISRVConstructorWithInfo(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle);
	static void IMGUISRVDestructorWithInfo(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);
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
	ID3D12PipelineState* m_DebugDrawPipeline;
	HRESULT CreateGraphicsPipelines();
	void DestroyGraphicsPipelines();

	Matrix4x4 m_ProjectionMatrix;

	D3D12_CPU_DESCRIPTOR_HANDLE m_NullTextureDescriptorCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_NullTextureDescriptorGPUHandle;
	HRESULT CreateNullDescriptors();
	void DestroyNullDescriptors();

	PushConstants* m_PushConstants;
	void UploadPushConstants();

	HRESULT CreateDefaultBuffer(ID3D12Resource*& defaultBuffer, ID3D12Resource*& gpuUploadBuffer, const void* data, const size_t& sizeBytes);

	void PrepareDefaultModelRender();

	float m_ProjectionFOV;

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

	bool SetDebugDrawMode();
	bool SetDefaultDrawMode();

	HRESULT UpdateWorldMatrix(const Matrix4x4& worldMatrix);
	HRESULT UpdateLightingBuffer(const LightBuffer& lb);
	HRESULT UpdateConstantBuffer(const ConstantBuffer& cb);

	bool BindGenericBufferData(GenericBuffer& genericBuffer, const void* buffer, const size_t& bufferLength);
	bool BindVertexData(VertexBuffer& vertexBuffer, const void* buffer, const size_t& bufferLength);
	bool BindIndexData(IndexBuffer& vertexBuffer, const void* buffer, const size_t& bufferLength, const bool& isShortIndex = false);

	TextureRef BindTextureData(const int& indexToBindTo, const void* data, const size_t& len, const Vector2& dimensions);

	bool Initialise(HWND windowHandle);
	bool InitialiseIMGUI();
	bool Shutdown();
	bool ShutdownIMGUI();

	const int& GetWindowWidth() const;
	const int& GetWindowHeight() const;

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetNullTextureDescriptorCPUHandle() const;
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetNullTextureDescriptorGPUHandle() const;
	bool AssignTextureToTextureSlot(const TEXTURE_TYPE_SLOT& textureSlot, const TextureRef& texture);

	const Matrix4x4& GetProjectionMatrix() const;
	Matrix4x4& GetProjectionMatrix();
	const Matrix4x4 GetViewMatrix() const;

	bool ResizeSwapchain(const int& newWidth, const int& newHeight);
	void PostAssetInitialisation();

	void Render(const ModelRef& mesh, const Matrix4x4& worldMatrix);

	void ClearFrame();
	HRESULT FlushCommandQueue();
	void PresentFrame();
	void RenderIMGUIFrame();

	// Inherited via IIMGUIRenderable
	void OnIMGUIRender() override;
};