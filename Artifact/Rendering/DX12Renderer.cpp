#include "pch.h"
#include "DX12Renderer.h"
#include <Rendering/ConstantBuffer.h>
#include <Rendering/Geometry/Mesh.h>
#include <Rendering/Vertex.h>
#include <Rendering/IndexBuffer.h>
#include <Rendering/VertexBuffer.h>
#include <functional>
#include "Geometry/Model.h"

#define FAILED_RETURN(hr) if(FAILED(hr)) return !FAILED(hr);

ID3D12DescriptorHeap* DX12Renderer::m_MainStorageSRVHeap = nullptr;
ID3D12DescriptorHeap* DX12Renderer::m_DrawCopySRVHeap = nullptr;
ID3D12DescriptorHeap* DX12Renderer::m_IMGUISRVHeap = nullptr;
ID3D12Device* DX12Renderer::m_Device = nullptr;
UINT DX12Renderer::m_RTVDescriptorHeapSize;
UINT DX12Renderer::m_DSVDescriptorHeapSize;
UINT DX12Renderer::m_CBVSRVDescriptorHeapSize;
int DX12Renderer::m_MainStorageSRVHeapDescriptorEndIndex = 0;
int DX12Renderer::m_DrawSRVHeapDescriptorEndIndex = 0;
int DX12Renderer::m_IMGUISRVHeapDescriptorEndIndex = 0;

DX12Renderer::DX12Renderer() : Renderer()
{
    m_UseMultisampling = false;
    m_ProjectionFOV = 90.0f;
    m_NullTextureDescriptor = {};
    m_MainStorageSRVHeap = nullptr;
    m_CBVHeaps = nullptr;
    m_DXGIFactory = nullptr;
    m_Device = nullptr;
    m_InfoQueue = nullptr;
    m_Fence = nullptr;
    m_RTVDescriptorHeapSize = 0;
    m_DSVDescriptorHeapSize = 0;
    m_CBVSRVDescriptorHeapSize = 0;
    m_m4xMSAAQuality = 0;
    m_CommandQueue = nullptr;
    m_CommandList = nullptr;
    m_CommandAllocator = nullptr;
    m_SwapChain = nullptr;
    m_CurrentBackbufferIndex = 0;
    m_RTVHeap = nullptr;
    m_DSVHeap = nullptr;
    m_CurrentFenceIndex = 0;
    m_SwapchainBuffers = nullptr;
    m_Viewport = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_ScissorRect = {};
    m_DepthStencilBuffer = nullptr;
    m_DefaultPixelShaderBlob = nullptr;
    m_DefaultVertexShaderBlob = nullptr;
    m_DefaultPipeline = nullptr;
    m_RootSignature = nullptr;
    m_PushConstants = nullptr;
    m_CBVHeaps = new ID3D12DescriptorHeap * [m_SwapChainBufferCount];
    m_ConstantBufferAddressArray = new char* [m_SwapChainBufferCount];
    m_ConstantBufferGPUUploaderArray = new ID3D12Resource * [m_SwapChainBufferCount];
    m_LightBufferAddressArray = new char* [m_SwapChainBufferCount];
    m_LightBufferGPUUploaderArray = new ID3D12Resource * [m_SwapChainBufferCount];

    for (size_t i = 0; i < m_SwapChainBufferCount; i++)
    {
        m_ConstantBufferGPUUploaderArray[i] = nullptr;
        m_ConstantBufferAddressArray[i] = nullptr;
        m_LightBufferAddressArray[i] = nullptr;
        m_LightBufferGPUUploaderArray[i] = nullptr;
        m_CBVHeaps[i] = nullptr;
    }

    DirectX::XMStoreFloat4x4(&m_ProjectionMatrix, DirectX::XMMatrixIdentity());
}

DX12Renderer::~DX12Renderer()
{
    assert(IsInitialised() == false);

    if (m_ConstantBufferGPUUploaderArray != nullptr)
    {
        delete[] m_ConstantBufferGPUUploaderArray;
        m_ConstantBufferGPUUploaderArray = nullptr;
    }

    if (m_ConstantBufferAddressArray != nullptr)
    {
        delete[] m_ConstantBufferAddressArray;
        m_ConstantBufferAddressArray = nullptr;
    }

    if (m_LightBufferGPUUploaderArray != nullptr)
    {
        delete[] m_LightBufferGPUUploaderArray;
        m_LightBufferGPUUploaderArray = nullptr;
    }

    if (m_LightBufferAddressArray != nullptr)
    {
        delete[] m_LightBufferAddressArray;
        m_LightBufferAddressArray = nullptr;
    }

    if (m_CBVHeaps != nullptr)
    {
        delete[] m_CBVHeaps;
        m_CBVHeaps = nullptr;
    }

    if (m_PushConstants != nullptr)
    {
        delete m_PushConstants;
        m_PushConstants = nullptr;
    }
}

ID3D12CommandQueue* DX12Renderer::GetCommandQueue()
{
    assert(m_IsInitialised);
    return m_CommandQueue;
}

const ID3D12CommandQueue* DX12Renderer::GetCommandQueue() const
{
    assert(m_IsInitialised);
    return m_CommandQueue;
}

HRESULT DX12Renderer::ResetCommandList()
{
    return m_CommandList->Reset(m_CommandAllocator, m_DefaultPipeline);
}

ID3D12GraphicsCommandList* DX12Renderer::GetCommandList()
{
    assert(m_IsInitialised);
    return m_CommandList;
}

const ID3D12GraphicsCommandList* DX12Renderer::GetCommandList() const
{
    assert(m_IsInitialised);
    return m_CommandList;
}

const ID3D12Device* DX12Renderer::GetDevice() const
{
    assert(m_IsInitialised);
    return m_Device;
}

ID3D12Device* DX12Renderer::GetDevice()
{
    assert(m_IsInitialised);
    return m_Device;
}

bool DX12Renderer::Initialise(HWND windowHandle)
{
    if (IsInitialised())
    {
        printf("Calling initialise on a  object that already exists\n");
        return false;
    }

    if (windowHandle == NULL)
    {
        printf("Passing an invalid window handle\n");
        return false;
    }

    HRESULT hr = S_OK;
    m_IsInitialised = true;
    m_WindowHandle = windowHandle;

    hr = CreateDeviceAndFactory();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr);

    hr = CreateFence();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr);

    hr = DetermineMultisamplingDetails();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr);

    hr = CreateCommandObjects();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = CreateSwapChain();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = CreateDescriptorHeaps();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = CreateRenderTargetViews();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = CreateDepthStencilBuffer();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = UpdateViewportAndScissorRect();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = CreateInputLayout();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = CreateConstantBufferHeap();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = CreateConstantBuffers();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = CreateRootSignatureAndDescriptorTable();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = FindAndCreateShaders();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = CreateGraphicsPipelines();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = CreateNullDescriptors();
    m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    m_PushConstants = new PushConstants();

    m_IsInitialised &= InitialiseIMGUI();
    
    if (IsInitialised() == false)
    {
        m_WindowHandle = NULL;
        printf("Failed to initialise \n");
    }
    else
    {
        printf("Initialised renderer.");
    }

    return m_IsInitialised;
}

bool DX12Renderer::InitialiseIMGUI()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    io.WantCaptureKeyboard = true;
    io.WantCaptureMouse = true;
    io.ConfigDockingWithShift = true;
    io.ConfigDockingTransparentPayload = true;

    io.MouseDrawCursor = true;
    io.DisplaySize.x = (float)m_WindowWidth;
    io.DisplaySize.y = (float)m_WindowHeight;

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    bool win32Init = ImGui_ImplWin32_Init(m_WindowHandle);

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = m_Device;
    init_info.CommandQueue = m_CommandQueue;
    init_info.NumFramesInFlight = m_SwapChainBufferCount;
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;

    init_info.SrvDescriptorHeap = m_MainStorageSRVHeap;
    init_info.SrvDescriptorAllocFn = IMGUISRVConstructorWithInfo;
    init_info.SrvDescriptorFreeFn = IMGUISRVDestructorWithInfo;

    bool dx12Init = ImGui_ImplDX12_Init(&init_info);
   
    return (win32Init && dx12Init);
}

void DX12Renderer::IMGUISRVConstructorWithInfo(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle)
{
    UNREFERENCED_PARAMETER(info);

    UINT offset = UINT64(m_MainStorageSRVHeapDescriptorEndIndex) * UINT64(m_CBVSRVDescriptorHeapSize);

    if (cpuHandle)
    {
        cpuHandle->ptr = info->SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + offset;
    }

    if (gpuHandle)
    {
        gpuHandle->ptr = info->SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + offset;
    }
    m_MainStorageSRVHeapDescriptorEndIndex++;

    //CD3DX12_CPU_DESCRIPTOR_HANDLE destDescriptor(m_IMGUISRVHeap->GetCPUDescriptorHandleForHeapStart());
    //destDescriptor.Offset(m_IMGUISRVHeapDescriptorEndIndex * m_CBVSRVDescriptorHeapSize);
    //m_Device->CopyDescriptorsSimple(1, destDescriptor, *cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    //m_IMGUISRVHeapDescriptorEndIndex++;
}

void DX12Renderer::GetNewDescriptorHandleFromMainSRVHeap(D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle)
{
    if (cpuHandle)
    {
        cpuHandle->ptr = m_MainStorageSRVHeap->GetCPUDescriptorHandleForHeapStart().ptr + (UINT64(m_MainStorageSRVHeapDescriptorEndIndex) * UINT64(m_CBVSRVDescriptorHeapSize));
    }

    if (gpuHandle)
    {
        gpuHandle->ptr = m_MainStorageSRVHeap->GetGPUDescriptorHandleForHeapStart().ptr + (UINT64(m_MainStorageSRVHeapDescriptorEndIndex) * UINT64(m_CBVSRVDescriptorHeapSize));
    }

    m_MainStorageSRVHeapDescriptorEndIndex++;
}

void DX12Renderer::IMGUISRVDestructorWithInfo(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
    UNREFERENCED_PARAMETER(info);
    UNREFERENCED_PARAMETER(cpuHandle);
    UNREFERENCED_PARAMETER(gpuHandle);
    //auto cpuHeapHandle = m_IMGUISRVHeap->GetCPUDescriptorHandleForHeapStart();
    //auto gpuHeapStart = m_IMGUISRVHeap->GetGPUDescriptorHandleForHeapStart();
    //int cpu_idx = (int)((cpuHandle.ptr - cpuHeapHandle.ptr) / m_CBVSRVDescriptorHeapSize);
    //int gpu_idx = (int)((gpuHandle.ptr - gpuHeapStart.ptr) / m_CBVSRVDescriptorHeapSize);
}

bool DX12Renderer::Shutdown()
{
    if (!IsInitialised())
    {
        printf("Calling shutdown on a DX12Renderer object that doesn't exist.");
        return false;
    }

    ShutdownIMGUI();

    if (m_PushConstants != nullptr)
    {
        delete m_PushConstants;
        m_PushConstants = nullptr;
    }

    DestroyNullDescriptors();
    DestroyGraphicsPipelines();
    DestroyLoadedShaders();
    DestroyRootSignatureAndDescriptorTable();
    DestroyConstantBuffers();
    DestroyConstantBufferHeap();
    DestroyInputLayout();
    DestroyDepthStencilBuffer();
    DestroyRenderTargetViews();
    DestroyDescriptorHeaps();
    DestroySwapChain();
    DestroyCommandObjects();
    ClearMultisamplingDetails();
    DestroyFence();
    DestroyDeviceAndFactory();

    m_WindowHandle = NULL;

    return true;
}

bool DX12Renderer::ShutdownIMGUI()
{
    // Cleanup
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    return true;
}

const int& DX12Renderer::GetWindowWidth() const
{
    return m_WindowWidth;
}

const int& DX12Renderer::GetWindowHeight() const
{
    return m_WindowHeight;
}

const D3D12_CPU_DESCRIPTOR_HANDLE& DX12Renderer::GetNullTextureDescriptor() const
{
    return m_NullTextureDescriptor;
}

bool DX12Renderer::AssignTextureToTextureSlot(const TEXTURE_TYPE_SLOT& textureSlot, const TextureRef& texture)
{
    int slotIndex = (int)textureSlot;
    if (slotIndex < 0 || slotIndex >= TEXTURE_TYPE_SLOT::TEXTURE_TYPE_COUNT)
    {
        printf("Trying to assign a texture to an invalid slot.");
        return false;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE destDescriptor(m_DrawCopySRVHeap->GetCPUDescriptorHandleForHeapStart());
    destDescriptor.Offset(slotIndex * GetSRVDescriptorHeapSize());

    if (m_PushConstants == nullptr)
    {
        printf("Trying to use an invalid push constant buffer.\n");
        return false;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE handle = GetNullTextureDescriptor();

    if (texture != nullptr)
    {
        if (texture->IsLoaded())
        {
            handle = texture->GetCPUHandle();
        }
    }

    m_CommandList->SetGraphicsRootDescriptorTable(3, texture->GetGPUHandle());
    //m_Device->CopyDescriptorsSimple(1, destDescriptor, handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    UploadPushConstants();

    return true;
}

const Matrix4x4& DX12Renderer::GetProjectionMatrix() const
{
    return m_ProjectionMatrix;
}

Matrix4x4& DX12Renderer::GetProjectionMatrix()
{
    return m_ProjectionMatrix;
}

const Matrix4x4 DX12Renderer::GetViewMatrix() const
{
    return m_Camera.GetViewMatrix();
}

bool DX12Renderer::ResizeSwapchain(const int& newWidth, const int& newHeight)
{
    assert(m_IsInitialised);

    if (m_SwapChain == nullptr)
    {
        printf("Swapchain is invalid for resizing.\n");
        return SUCCEEDED(E_FAIL);
    }

    DestroyRenderTargetViews();
    DestroyDepthStencilBuffer();

    //Passing unknown format to retain the same format as the current buffers.
    HRESULT result = m_SwapChain->ResizeBuffers(m_SwapChainBufferCount, newWidth, newHeight, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

    if (FAILED(result))
    {
        printf("Serious failure while resizing swapchain buffers\n");
        return SUCCEEDED(result);
    }

    RECT rect{};
    GetClientRect(m_WindowHandle, &rect);
    m_WindowWidth = rect.right - rect.left;
    m_WindowHeight = rect.bottom - rect.top;

    result = CreateRenderTargetViews();

    if (FAILED(result))
    {
        printf("Failed to recreate render target views after swapchain resizing.\n");
        return SUCCEEDED(result);
    }

    result = CreateDepthStencilBuffer();

    if (FAILED(result))
    {
        printf("Failed to recreate depth stencil view after swapchain resizing.\n");
        return SUCCEEDED(result);
    }

    m_CurrentBackbufferIndex = 0;

    result = UpdateViewportAndScissorRect();

    if (FAILED(result))
    {
        printf("Failed to update viewport or scissor rect during swapchain resize.\n");
        return SUCCEEDED(result);
     }

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = (float)m_WindowWidth;
    io.DisplaySize.y = (float)m_WindowHeight;

    return SUCCEEDED(result);
}

void DX12Renderer::PostAssetInitialisation()
{ 
    //Setting to closed as the first refernce to the command list will open it.
    if (m_CommandList)
    {
        HRESULT hr = m_CommandList->Close();
        assert(SUCCEEDED(hr));
    }

    m_Camera.Move(Vector3(0.0f, 0.0f, -15.0f));
}

void DX12Renderer::PrepareDefaultModelRender()
{
    m_CommandList->SetGraphicsRootSignature(m_RootSignature);

    ID3D12DescriptorHeap* heaps[] = { m_MainStorageSRVHeap };
    m_CommandList->SetDescriptorHeaps(_countof(heaps), heaps);

    m_CommandList->SetPipelineState(m_DefaultPipeline);

    static ConstantBuffer cb = {};
    cb.Projection = m_ProjectionMatrix;
    cb.View = m_Camera.GetViewMatrix();
    UpdateConstantBuffer(cb);
}

void DX12Renderer::Render(const ModelRef& model, const Matrix4x4& worldMatrix)
{
    if (model->IsLoaded() == false)
    {
        printf("Trying to draw a model that isn't loaded.\n");
        return;
    }

    PrepareDefaultModelRender();

    for (auto& mesh : model->GetMeshes())
    {
        Matrix4x4 finalWorld{};
        DirectX::XMStoreFloat4x4(&finalWorld, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&worldMatrix), DirectX::XMLoadFloat4x4(&mesh->GetOffsetMatrix())));
        DirectX::XMStoreFloat4x4(&m_PushConstants->World, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&finalWorld)));
        UploadPushConstants();

        for (auto& texture : mesh->GetTextures())
        {
            AssignTextureToTextureSlot(TEXTURE_TYPE_DIFFUSE, texture);
        }

        GetCommandList()->IASetVertexBuffers(0, 1, &mesh->GetVertexBuffer().GetBufferView());

        MESH_TOPOLOGY topology = mesh->GetTopologyType();

        if (topology == MESH_TOPOLOGY_UNDEFINED)
        {
            printf("Trying to draw a mesh with an undefined topology.\n");
            return;
        }

        GetCommandList()->IASetPrimitiveTopology((D3D12_PRIMITIVE_TOPOLOGY)topology);

        if (mesh->GetIndexBuffer().IsLoaded())
        {
            GetCommandList()->IASetIndexBuffer(&mesh->GetIndexBuffer().GetBufferView());
            GetCommandList()->DrawIndexedInstanced(mesh->GetIndexBuffer().GetElementCount(), 1, 0, 0, 0);
        }
        else
        {
            GetCommandList()->DrawInstanced(mesh->GetVertexBuffer().GetElementCount(), 1, 0, 0);
        }
    }
}

void DX12Renderer::BeginIMGUIFrame()
{
    if (m_CommandList == nullptr)
    {
        printf("Error starting IMGUI frame, invalid command list.\n");
        return;
    }
    ID3D12DescriptorHeap* heaps[] = { m_MainStorageSRVHeap };
    m_CommandList->SetDescriptorHeaps(_countof(heaps), heaps);

    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX12_NewFrame();

    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_::ImGuiDockNodeFlags_PassthruCentralNode);
}

void DX12Renderer::EndIMGUIFrame()
{
    if (m_CommandList == nullptr)
    {
        printf("Error ending IMGUI frame, invalid command list.\n");
        return;
    }

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_CommandList);
}

HRESULT DX12Renderer::CreateDeviceAndFactory()
{
    HRESULT result = E_FAIL;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_1;
    UINT factoryFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
    //Enabling D3D12 Debug layer.
    ID3D12Debug* debug;
    result = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));

    if (FAILED(result))
    {
        printf("Failed to enable debug layer within debug build.\n");
    }

    if (debug != nullptr && result == S_OK)
    {
        debug->EnableDebugLayer();
        // Enable additional debug layers.
        factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    result = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_DXGIFactory));
    if (FAILED(result))
    {
        printf("Failed to create D3D12 Factorys.\n");
        return result;
    }

    //Create hardware device
    result = D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&m_Device));
    if (FAILED(result))
    {
        printf("Failed to create D3D12 Device.\n");
        return result;
    }

    m_Device->SetName(L"Graphics Device");

#if defined(DEBUG) || defined(_DEBUG)
    m_Device->QueryInterface(IID_PPV_ARGS(&m_InfoQueue));
    m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
    m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
#endif

    //Cache descriptor sizes for later.
    m_RTVDescriptorHeapSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_DSVDescriptorHeapSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_CBVSRVDescriptorHeapSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    return result;
}

void DX12Renderer::DestroyDeviceAndFactory()
{
    if (m_InfoQueue != nullptr)
    {
        m_InfoQueue->Release();
        m_InfoQueue = nullptr;
    }

    if (m_Device != nullptr)
    {
        m_Device->Release();
        m_Device = nullptr;
    }

    if (m_DXGIFactory != nullptr)
    {
        m_DXGIFactory->Release();
        m_DXGIFactory = nullptr;
    }
}

HRESULT DX12Renderer::CreateFence()
{
    HRESULT result = E_FAIL;

    if (m_Device == nullptr)
        return E_POINTER;

    D3D12_FENCE_FLAGS fenceFlag = D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE;
    result = m_Device->CreateFence(0, fenceFlag, IID_PPV_ARGS(&m_Fence));

    m_Fence->SetName(L"Default Fence");

    if (FAILED(result))
    {
        printf("Failed to create fence from ID3D12Device.\n");
        return result;
    }

    return result;
}

void DX12Renderer::DestroyFence()
{
    if (m_Fence != nullptr)
    {
        m_Fence->Release();
        m_Fence = nullptr;
    }

    m_RTVDescriptorHeapSize = 0;
    m_DSVDescriptorHeapSize = 0;
    m_CBVSRVDescriptorHeapSize = 0;
}

HRESULT DX12Renderer::DetermineMultisamplingDetails()
{
    HRESULT result = E_FAIL;

    if (m_Device == nullptr)
        return E_POINTER;

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS multisampleQualityLevels{};
    multisampleQualityLevels.Format = m_BackbufferFormat;
    multisampleQualityLevels.SampleCount = 4;
    multisampleQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS::D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    multisampleQualityLevels.NumQualityLevels = 0;

    result = m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &multisampleQualityLevels, sizeof(multisampleQualityLevels));

    if (FAILED(result) || multisampleQualityLevels.NumQualityLevels <= 0)
    {
        printf("Failed to locate multisampling quality levels from ID3D12Device.\n");
        return result;
    }

    m_m4xMSAAQuality = multisampleQualityLevels.NumQualityLevels;

    return result;
}

void DX12Renderer::ClearMultisamplingDetails()
{
    m_m4xMSAAQuality = 0;
}

HRESULT DX12Renderer::CreateCommandObjects()
{
    HRESULT result = E_FAIL;

    if (m_Device == nullptr)
        return E_POINTER;

    D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
    commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
    commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;

    result = m_Device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_CommandQueue));

    if (FAILED(result))
    {
        printf("Failed to create command queue from ID3D12Device.\n");
        return result;
    }

    m_CommandQueue->SetName(L"Command Queue");

    result = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));

    if (FAILED(result))
    {
        printf("Failed to create command allocator from ID3D12Device.\n");
        return result;
    }

    m_CommandAllocator->SetName(L"Command Allocator");

    result = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator, nullptr, IID_PPV_ARGS(&m_CommandList));

    if (FAILED(result))
    {
        printf("Failed to create command list from ID3D12Device.\n");
        return result;
    }

    m_CommandList->SetName(L"Command List");

    return result;
}

void DX12Renderer::DestroyCommandObjects()
{
    if (m_CommandList != nullptr)
    {
        m_CommandList->Release();
        m_CommandList = nullptr;
    }

    if (m_CommandAllocator != nullptr)
    {
        m_CommandAllocator->Release();
        m_CommandAllocator = nullptr;
    }

    if (m_CommandQueue != nullptr)
    {
        m_CommandQueue->Release();
        m_CommandQueue = nullptr;
    }
}

HRESULT DX12Renderer::CreateSwapChain()
{
    HRESULT result = E_FAIL;

    if (m_DXGIFactory == nullptr || m_CommandQueue == nullptr)
        return E_POINTER;

    DXGI_SWAP_CHAIN_FLAG swapChainFlags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    RECT rect{};
    GetClientRect(m_WindowHandle, &rect);
    m_WindowWidth = rect.right - rect.left;
    m_WindowHeight = rect.bottom - rect.top;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_WindowWidth;
    swapChainDesc.Height = m_WindowHeight;
    swapChainDesc.Format = m_BackbufferFormat;
    swapChainDesc.Stereo = false;
    swapChainDesc.BufferCount = m_SwapChainBufferCount;
    swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    swapChainDesc.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = swapChainFlags;
    swapChainDesc.SampleDesc.Count = m_UseMultisampling ? 4 : 1;
    swapChainDesc.SampleDesc.Quality = m_UseMultisampling ? (m_m4xMSAAQuality - 1) : 0;

    result = m_DXGIFactory->CreateSwapChainForHwnd(
        m_CommandQueue,
        m_WindowHandle,
        &swapChainDesc,
        nullptr, nullptr, &m_SwapChain);

    if (FAILED(result))
    {
        printf("Failed to create swap chain.\n");
        return result;
    }

    return result;
}

void DX12Renderer::DestroySwapChain()
{
    if (m_SwapChain != nullptr)
    {
        m_SwapChain->Release();
        m_SwapChain = nullptr;
    }
}

HRESULT DX12Renderer::CreateDescriptorHeaps()
{
    HRESULT result = E_FAIL;

    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDescriptorHeapDesc.NumDescriptors = m_SwapChainBufferCount;
    rtvDescriptorHeapDesc.NodeMask = 0;
    rtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    result = m_Device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&m_RTVHeap));

    if (FAILED(result))
    {
        printf("Failed to create RTV descriptor heap.\n");
        return result;
    }

    m_RTVHeap->SetName(L"Render Target Heap");

    D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc{};
    dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvDescriptorHeapDesc.NumDescriptors = 1;
    dsvDescriptorHeapDesc.NodeMask = 0;
    dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    m_Device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&m_DSVHeap));

    if (FAILED(result))
    {
        printf("Failed to create DSV descriptor heap.\n");
        return result;
    }

    m_DSVHeap->SetName(L"Depth Stencil Heap");

    D3D12_DESCRIPTOR_HEAP_DESC mainSRVDescriptorHeapDesc{};
    mainSRVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    mainSRVDescriptorHeapDesc.NumDescriptors = MAX_LOADABLE_TEXTURES;
    mainSRVDescriptorHeapDesc.NodeMask = 0;
    mainSRVDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    result = m_Device->CreateDescriptorHeap(&mainSRVDescriptorHeapDesc, IID_PPV_ARGS(&m_MainStorageSRVHeap));

    if (FAILED(result))
    {
        printf("Failed to create main SRV descriptor heap.\n");
        return result;
    }

    m_MainStorageSRVHeap->SetName(L"Main CBV/SRV/UAV Heap");

    D3D12_DESCRIPTOR_HEAP_DESC drawDescriptorHeapDesc{};
    drawDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    drawDescriptorHeapDesc.NumDescriptors = MAX_LOADABLE_TEXTURES;
    drawDescriptorHeapDesc.NodeMask = 0;
    drawDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    m_Device->CreateDescriptorHeap(&drawDescriptorHeapDesc, IID_PPV_ARGS(&m_DrawCopySRVHeap));

    if (FAILED(result))
    {
        printf("Failed to create per object SRV descriptor heap.\n");
        return result;
    }

    m_DrawCopySRVHeap->SetName(L"Draw Call CBV/SRV/UAV Heap");

    D3D12_DESCRIPTOR_HEAP_DESC IMGUIDescriptorHeapDesc{};
    IMGUIDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    IMGUIDescriptorHeapDesc.NumDescriptors = MAX_LOADABLE_TEXTURES;
    IMGUIDescriptorHeapDesc.NodeMask = 0;
    IMGUIDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    m_Device->CreateDescriptorHeap(&IMGUIDescriptorHeapDesc, IID_PPV_ARGS(&m_IMGUISRVHeap));

    if (FAILED(result))
    {
        printf("Failed to create IMGUI SRV descriptor heap.\n");
        return result;
    }

    m_IMGUISRVHeap->SetName(L"IMGUI CBV/SRV/UAV Heap");

    m_DrawSRVHeapDescriptorEndIndex = 0;
    m_MainStorageSRVHeapDescriptorEndIndex = 0;
    m_IMGUISRVHeapDescriptorEndIndex = 0;

    return result;
}

void DX12Renderer::DestroyDescriptorHeaps()
{
    if (m_RTVHeap != nullptr)
    {
        m_RTVHeap->Release();
        m_RTVHeap = nullptr;
    }

    if (m_DSVHeap != nullptr)
    {
        m_DSVHeap->Release();
        m_DSVHeap = nullptr;
    }

    if (m_MainStorageSRVHeap != nullptr)
    {
        m_MainStorageSRVHeap->Release();
        m_MainStorageSRVHeap = nullptr;
        m_MainStorageSRVHeapDescriptorEndIndex = 0;
    }

    if (m_DrawCopySRVHeap != nullptr)
    {
        m_DrawCopySRVHeap->Release();
        m_DrawCopySRVHeap = nullptr;
        m_DrawSRVHeapDescriptorEndIndex = 0;
    }

    if (m_IMGUISRVHeap != nullptr)
    {
        m_IMGUISRVHeap->Release();
        m_IMGUISRVHeap = nullptr;
        m_IMGUISRVHeapDescriptorEndIndex = 0;
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Renderer::GetCurrentBackbufferView() const
{
    assert((m_RTVHeap != nullptr));
    D3D12_CPU_DESCRIPTOR_HANDLE handle = D3D12_CPU_DESCRIPTOR_HANDLE();
    handle.ptr = m_RTVHeap->GetCPUDescriptorHandleForHeapStart().ptr + (m_CurrentBackbufferIndex * m_RTVDescriptorHeapSize);
    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Renderer::GetDepthStencilBufferView() const
{
    assert((m_DSVHeap != nullptr));
    return m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
}

HRESULT DX12Renderer::CreateRenderTargetViews()
{
    HRESULT result = E_FAIL;

    if (m_Device == nullptr || m_SwapChain == nullptr || m_RTVHeap == nullptr)
    {
        printf("Tried to create render target views with invalid pointer value.\n");
        return E_POINTER;
    }

    if (m_SwapchainBuffers != nullptr)
    {
        printf("Trying to create swap chain buffer list when they already exist.\n");
        return E_POINTER;
    }

    m_SwapchainBuffers = new ID3D12Resource * [m_SwapChainBufferCount];

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();

    for (int i = 0; i < m_SwapChainBufferCount; i++)
    {
        result = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapchainBuffers[i]));

        if (FAILED(result))
        {
            printf("Failed to get swap chain buffer %ui from swap chain.\n", i);
            return result;
        }

        m_Device->CreateRenderTargetView(m_SwapchainBuffers[i], nullptr, rtvHandle);
        std::wstring n = L"Swapchain Buffer" + std::to_wstring(i);
        m_SwapchainBuffers[i]->SetName(n.c_str());

        rtvHandle.ptr += m_RTVDescriptorHeapSize;
    }

    return result;
}

void DX12Renderer::DestroyRenderTargetViews()
{
    for (size_t i = 0; i < m_SwapChainBufferCount; i++)
    {
        m_SwapchainBuffers[i]->Release();
        m_SwapchainBuffers[i] = nullptr;
    }

    delete[] m_SwapchainBuffers;
    m_SwapchainBuffers = nullptr;
}

HRESULT DX12Renderer::CreateDepthStencilBuffer()
{
    HRESULT result = E_FAIL;

    if (m_Device == nullptr || m_CommandList == nullptr)
    {
        printf("Tried to create depth stencil buffer with an invalid device.\n");
        return E_POINTER;
    }

    D3D12_RESOURCE_DESC depthStencilDesc{};
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = m_WindowWidth;
    depthStencilDesc.Height = m_WindowHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = m_DepthStencilBufferFormat;
    depthStencilDesc.SampleDesc.Count = m_UseMultisampling ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = m_UseMultisampling ? (m_m4xMSAAQuality - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Color[0] = 1.0f;
    clearValue.Color[1] = 1.0f;
    clearValue.Color[2] = 1.0f;
    clearValue.Color[3] = 1.0f;
    clearValue.Format = m_DepthStencilBufferFormat;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    result = m_Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&m_DepthStencilBuffer));

    if (FAILED(result))
    {
        printf("Failed to create commited device resource for depth stencil buffer\n.");
        return result;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = m_DepthStencilBufferFormat;
    dsvDesc.Texture2D.MipSlice = 0;

    m_Device->CreateDepthStencilView(m_DepthStencilBuffer, nullptr, GetDepthStencilBufferView());

    m_DepthStencilBuffer->SetName(L"Depth Stencil Buffer");

    return result;
}

void DX12Renderer::DestroyDepthStencilBuffer()
{
    if (m_DepthStencilBuffer != nullptr)
    {
        m_DepthStencilBuffer->Release();
        m_DepthStencilBuffer = nullptr;
    }
}

HRESULT DX12Renderer::UpdateViewportAndScissorRect()
{
    if (m_CommandList == nullptr)
    {
        printf("Tried to set a viewport with an invalid command list.\n");
        return E_POINTER;
    }

    m_Viewport.TopLeftX = 0.0f;
    m_Viewport.TopLeftY = 0.0f;
    m_Viewport.Width = (float)m_WindowWidth;
    m_Viewport.Height = (float)m_WindowHeight;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

    m_ScissorRect.left = 0;
    m_ScissorRect.top = 0;
    m_ScissorRect.right = m_WindowWidth;
    m_ScissorRect.bottom = m_WindowHeight;

    float aspectRatio = (float)m_WindowWidth / (float)m_WindowHeight;    
    float fovRadians = m_ProjectionFOV * DEGREES_TO_RADIANS;
    DirectX::XMStoreFloat4x4(&m_ProjectionMatrix, DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, DEFAULT_NEAR_PLANE, DEFAULT_FAR_PLANE)));

    return S_OK;
}

HRESULT DX12Renderer::FlushCommandQueue()
{
    HRESULT result = E_FAIL;
    assert(m_IsInitialised);
    m_CurrentFenceIndex++;
    result = m_CommandQueue->Signal(m_Fence, m_CurrentFenceIndex);

    if (FAILED(result))
    {
        printf("Command Queue failed to signal fence.\n");
        return result;
    }

    if (m_Fence->GetCompletedValue() < m_CurrentFenceIndex)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, "Fence", 0, EVENT_ALL_ACCESS);
        result = m_Fence->SetEventOnCompletion(m_CurrentFenceIndex, eventHandle);

        if (FAILED(result))
        {
            printf("Failed to set signal event for fence.\n");
            return result;
        }

        if (eventHandle != NULL)
        {
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }
    }

    return result;
}

HRESULT DX12Renderer::CreateConstantBuffers()
{
    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    ConstantBuffer emptyCb = ConstantBuffer();
    LightBuffer emptyLb = LightBuffer();

    HRESULT result = E_FAIL;

    for (size_t i = 0; i < m_SwapChainBufferCount; i++)
    {
        //Creating GPU upload buffer.
        CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES::CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RANGE readRange(0, 0);

        //Creating per-frame Constant Buffer
        CD3DX12_RESOURCE_DESC cbResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantBuffer));

        result = m_Device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &cbResourceDesc,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_ConstantBufferGPUUploaderArray[i]));

        if (FAILED(result))
        {
            printf("Failed to create commited resource for constant buffer's GPU upload buffer.\n");
            return result;
        }

        std::wstring name = L"Constant Buffer GPU Upload Heap " + std::to_wstring(i);
        m_ConstantBufferGPUUploaderArray[i]->SetName(name.c_str());

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
        cbvDesc.BufferLocation = m_ConstantBufferGPUUploaderArray[i]->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (sizeof(ConstantBuffer) + 255) & ~255;

        m_Device->CreateConstantBufferView(&cbvDesc, m_CBVHeaps[i]->GetCPUDescriptorHandleForHeapStart());

        result = m_ConstantBufferGPUUploaderArray[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_ConstantBufferAddressArray[i]));

        if (FAILED(result) || m_ConstantBufferAddressArray[i] == nullptr)
        {
            printf("Failed to map constant buffer's GPU upload heap address.\n");
            return result;
        }

        memcpy(m_ConstantBufferAddressArray[i], &emptyCb, sizeof(ConstantBuffer));

        //--------------------------------------------------------------------//

        //Creating per-frame Light Buffer
        CD3DX12_RESOURCE_DESC lbResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(LightBuffer));

        result = m_Device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &lbResourceDesc,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_LightBufferGPUUploaderArray[i]));

        if (FAILED(result))
        {
            printf("Failed to create committed resource for light buffer's GPU upload buffer.\n");
            return result;
        }

        name = L"Light Buffer GPU Upload Heap " + std::to_wstring(i);
        m_LightBufferGPUUploaderArray[i]->SetName(name.c_str());

        D3D12_CONSTANT_BUFFER_VIEW_DESC lbvDesc{};
        lbvDesc.BufferLocation = m_LightBufferGPUUploaderArray[i]->GetGPUVirtualAddress();
        lbvDesc.SizeInBytes = (sizeof(LightBuffer) + 255) & ~255;

        CD3DX12_CPU_DESCRIPTOR_HANDLE lbHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CBVHeaps[i]->GetCPUDescriptorHandleForHeapStart());
        lbHandle.Offset(1, m_CBVSRVDescriptorHeapSize);
        m_Device->CreateConstantBufferView(&lbvDesc, lbHandle);

        result = m_LightBufferGPUUploaderArray[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_LightBufferAddressArray[i]));

        if (FAILED(result) || m_LightBufferAddressArray[i] == nullptr)
        {
            printf("Failed to map light buffer's GPU upload heap address.\n");
            return result;
        }

        memcpy(m_LightBufferAddressArray[i], &emptyLb, sizeof(LightBuffer));
    }

    return S_OK;
}

void DX12Renderer::DestroyConstantBuffers()
{
    CD3DX12_RANGE readRange(0, 0);

    for (size_t i = 0; i < m_SwapChainBufferCount; i++)
    {
        if (m_ConstantBufferGPUUploaderArray[i] != nullptr)
        {
            m_ConstantBufferGPUUploaderArray[i]->Unmap(0, &readRange);
            m_ConstantBufferGPUUploaderArray[i]->Release();
            m_ConstantBufferGPUUploaderArray[i] = nullptr;
        }

        if (m_ConstantBufferAddressArray[i] != nullptr)
        {
            m_ConstantBufferAddressArray[i] = nullptr;
        }

        if (m_LightBufferGPUUploaderArray[i] != nullptr)
        {
            m_LightBufferGPUUploaderArray[i]->Unmap(0, &readRange);
            m_LightBufferGPUUploaderArray[i]->Release();
            m_LightBufferGPUUploaderArray[i] = nullptr;
        }

        if (m_LightBufferAddressArray[i] != nullptr)
        {
            m_LightBufferAddressArray[i] = nullptr;
        }
    }
}

HRESULT DX12Renderer::CreateConstantBufferHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
    heapDesc.NumDescriptors = 2;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;

    HRESULT result = E_FAIL;

    for (size_t i = 0; i < m_SwapChainBufferCount; i++)
    {
        result = m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_CBVHeaps[i]));

        if (FAILED(result))
        {
            printf("Failed to create a constant buffer descriptor heap.\n");
            break;
        }

        std::wstring name = L"Constant Buffer Heap " + std::to_wstring(i);
        m_CBVHeaps[i]->SetName(name.c_str());
    }

    return result;
}

void DX12Renderer::DestroyConstantBufferHeap()
{
    for (size_t i = 0; i < m_SwapChainBufferCount; i++)
    {
        if (m_CBVHeaps[i] != nullptr)
        {
            m_CBVHeaps[i]->Release();
            m_CBVHeaps[i] = nullptr;
        }
    }
}

HRESULT DX12Renderer::CreateRootSignatureAndDescriptorTable()
{
    D3D12_ROOT_PARAMETER slotRootParameters[4]{};

    //CBV
    D3D12_ROOT_DESCRIPTOR perFrameConstantBufferDescriptor{};
    perFrameConstantBufferDescriptor.RegisterSpace = 0;
    perFrameConstantBufferDescriptor.ShaderRegister = 0;
    slotRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    slotRootParameters[0].Descriptor = perFrameConstantBufferDescriptor;
    slotRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    //Push Constants
    D3D12_ROOT_CONSTANTS rootConstants{};
    rootConstants.Num32BitValues = sizeof(PushConstants) / sizeof(UINT32);
    rootConstants.RegisterSpace = 0;
    rootConstants.ShaderRegister = 1;
    D3D12_ROOT_DESCRIPTOR pushConstantDescriptor{};
    pushConstantDescriptor.RegisterSpace = rootConstants.RegisterSpace;
    pushConstantDescriptor.ShaderRegister = rootConstants.ShaderRegister;
    slotRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    slotRootParameters[1].Descriptor = pushConstantDescriptor;
    slotRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
    slotRootParameters[1].Constants = rootConstants;

    //Lighting Constant Buffer
    D3D12_ROOT_DESCRIPTOR lightingConstantBufferDescriptor{};
    lightingConstantBufferDescriptor.RegisterSpace = 0;
    lightingConstantBufferDescriptor.ShaderRegister = 2;
    slotRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    slotRootParameters[2].Descriptor = lightingConstantBufferDescriptor;
    slotRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    //SRV Table
    D3D12_DESCRIPTOR_RANGE descriptorTableRange[1]{};
    descriptorTableRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorTableRange[0].NumDescriptors = 1;
    descriptorTableRange[0].BaseShaderRegister = 0;
    descriptorTableRange[0].RegisterSpace = 0;
    descriptorTableRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable{};
    descriptorTable.NumDescriptorRanges = _countof(descriptorTableRange);
    descriptorTable.pDescriptorRanges = &descriptorTableRange[0];
    slotRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    slotRootParameters[3].DescriptorTable = descriptorTable;

    D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[1]{};
    staticSamplerDesc[0].Filter = D3D12_FILTER::D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    staticSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplerDesc[0].MipLODBias = 0;
    staticSamplerDesc[0].MaxAnisotropy = 1;
    staticSamplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NEVER;
    staticSamplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR::D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    staticSamplerDesc[0].MinLOD = 0.0f;
    staticSamplerDesc[0].MaxLOD = FLT_MAX;
    staticSamplerDesc[0].ShaderRegister = 0;
    staticSamplerDesc[0].RegisterSpace = 0;
    staticSamplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.Init(
        _countof(slotRootParameters), slotRootParameters,
        _countof(staticSamplerDesc), &staticSamplerDesc[0],
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

    ID3DBlob* rootSigBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSigBlob, &errorBlob);

    if (FAILED(result))
    {
        printf("Failed to create serialised root signature.\n");
        OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));

        if (errorBlob != nullptr)
        {
            errorBlob->Release();
            errorBlob = nullptr;
        }

        return result;
    }

    if (errorBlob != nullptr)
    {
        errorBlob->Release();
        errorBlob = nullptr;
    }

    result = m_Device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

    if (FAILED(result))
    {
        printf("Failed to create root signature.\n");

        if (rootSigBlob != nullptr)
        {
            rootSigBlob->Release();
            rootSigBlob = nullptr;
        }

        return result;
    }

    if (rootSigBlob != nullptr)
    {
        rootSigBlob->Release();
        rootSigBlob = nullptr;
    }

    m_RootSignature->SetName(L"Root Signature");

    m_CommandList->SetGraphicsRootSignature(m_RootSignature);

    return S_OK;
}

void DX12Renderer::DestroyRootSignatureAndDescriptorTable()
{
    if (m_RootSignature != nullptr)
    {
        m_RootSignature->Release();
        m_RootSignature = nullptr;
    }
}

HRESULT DX12Renderer::ReadShaderData(const std::string& filename, ID3DBlob*& targetBlob)
{
    std::filesystem::path path = filename;

    if (std::filesystem::exists(path) == false)
    {
        printf("Failed to locate shader file '%s'\n", filename.c_str());
        return E_INVALIDARG;
    }

    if (targetBlob != nullptr)
    {
        printf("Trying to load shader into already loaded blob.\n");
        return E_POINTER;
    }

    //Load file.
    std::ifstream file(filename, std::ios::binary);
    //Seek end
    file.seekg(0, std::ios::end);
    //Get size from current position
    int size = (int)file.tellg();
    //Go back to the start for read.
    file.seekg(0, std::ios::beg);

    HRESULT result = D3DCreateBlob(size, &targetBlob);
    if (FAILED(result))
    {
        printf("Failed to create blob during shader object creation.\n");
        return E_POINTER;
    }

    file.read((char*)targetBlob->GetBufferPointer(), size);
    file.close();

    return S_OK;
}

HRESULT DX12Renderer::FindAndCreateShaders()
{
    HRESULT result = E_UNEXPECTED;

    result = ReadShaderData("PS_Standard.cso", m_DefaultPixelShaderBlob);

    if (FAILED(result))
    {
        //Error message displayed in function.
        return result;
    }

    result = ReadShaderData("VS_Standard.cso", m_DefaultVertexShaderBlob);

    if (FAILED(result))
    {
        //Error message displayed in function.
        return result;
    }

    return result;
}

void DX12Renderer::DestroyLoadedShaders()
{
    if (m_DefaultPixelShaderBlob != nullptr)
    {
        m_DefaultPixelShaderBlob->Release();
        m_DefaultPixelShaderBlob = nullptr;
    }

    if (m_DefaultVertexShaderBlob != nullptr)
    {
        m_DefaultVertexShaderBlob->Release();
        m_DefaultVertexShaderBlob = nullptr;
    }
}

HRESULT DX12Renderer::CreateGraphicsPipelines()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
    memset(&pipelineStateDesc, 0, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    pipelineStateDesc.pRootSignature = m_RootSignature;
    pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    pipelineStateDesc.SampleMask = UINT_MAX;
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateDesc.NodeMask = 0;
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0] = m_BackbufferFormat;
    pipelineStateDesc.DSVFormat = m_DepthStencilBufferFormat;
    pipelineStateDesc.SampleDesc.Count = 1;
    pipelineStateDesc.SampleDesc.Quality = 0;
    pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAGS::D3D12_PIPELINE_STATE_FLAG_NONE;

    HRESULT result = E_POINTER;
    if (m_DefaultVertexShaderBlob != nullptr && m_DefaultPixelShaderBlob != nullptr)
    {
        pipelineStateDesc.InputLayout.NumElements = (unsigned int)m_DefaultInputLayout.size();
        pipelineStateDesc.InputLayout.pInputElementDescs = m_DefaultInputLayout.data();
        pipelineStateDesc.VS.pShaderBytecode = m_DefaultVertexShaderBlob->GetBufferPointer();
        pipelineStateDesc.VS.BytecodeLength = m_DefaultVertexShaderBlob->GetBufferSize();
        pipelineStateDesc.PS.pShaderBytecode = m_DefaultPixelShaderBlob->GetBufferPointer();
        pipelineStateDesc.PS.BytecodeLength = m_DefaultPixelShaderBlob->GetBufferSize();
        result = m_Device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&m_DefaultPipeline));
    }

    if (FAILED(result))
    {
        printf("Failed to create default graphics pipeline state.\n");
        return result;
    }
    m_DefaultPipeline->SetName(L"Standard Graphics Pipeline");

    return result;
}

void DX12Renderer::DestroyGraphicsPipelines()
{
    if (m_DefaultPipeline != nullptr)
    {
        m_DefaultPipeline->Release();
        m_DefaultPipeline = nullptr;
    }
}

HRESULT DX12Renderer::CreateNullDescriptors()
{
    const DXGI_FORMAT textureFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    CD3DX12_SHADER_RESOURCE_VIEW_DESC srvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2D(textureFormat);
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = {};
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = {};

    GetNewDescriptorHandleFromMainSRVHeap(&srvCpuHandle, &srvGpuHandle);

    m_Device->CreateShaderResourceView(NULL, &srvDesc, srvCpuHandle);

    HRESULT result = ExecuteAndResetCommandList();

    if (FAILED(result))
    {
        printf("Failed to execute and reset command list during texture buffer creation.\n");
        return result;
    }

    m_NullTextureDescriptor = srvCpuHandle;

    return result;
}

void DX12Renderer::DestroyNullDescriptors()
{
    m_NullTextureDescriptor = {};
}

UINT DX12Renderer::GetSRVDescriptorHeapSize() const
{
    return m_CBVSRVDescriptorHeapSize;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Renderer::GetMainSRVDescriptorHeapStartCPU() const
{
    assert((m_MainStorageSRVHeap != nullptr));
    return m_MainStorageSRVHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE DX12Renderer::GetMainSRVDescriptorHeapStartGPU() const
{
    assert((m_MainStorageSRVHeap != nullptr));
    return m_MainStorageSRVHeap->GetGPUDescriptorHandleForHeapStart();
}

HRESULT DX12Renderer::ExecuteAndResetCommandList()
{
    HRESULT result = E_FAIL;

    result = m_CommandList->Close();
    if (FAILED(result))
    {
        printf("Failed to close command list during default buffer creation.\n");
        return result;
    }

    ID3D12CommandList* commandLists = { m_CommandList };
    m_CommandQueue->ExecuteCommandLists(1, &commandLists);

    result = FlushCommandQueue();
    if (FAILED(result))
    {
        printf("Failed to flush command queue during default buffer creation.\n");
        return result;
    }

    result = ResetCommandList();
    if (FAILED(result))
    {
        printf("Failed to reset command list during default buffer creation.\n");
        return result;
    }

    return result;
}

HRESULT DX12Renderer::CreateDefaultBuffer(ID3D12Resource*& defaultBuffer, ID3D12Resource*& gpuUploadBuffer, const void* data, const size_t& sizeBytes)
{
    assert((defaultBuffer == nullptr));
    assert((gpuUploadBuffer == nullptr));
    assert((data != nullptr));
    assert(sizeBytes > 0);

    CD3DX12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES::CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC defaultBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer((UINT64)sizeBytes);

    //Creating Default Buffer
    HRESULT result = m_Device->CreateCommittedResource(
        &defaultHeapProperties, D3D12_HEAP_FLAG_NONE,
        &defaultBufferResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&defaultBuffer));

    if (FAILED(result))
    {
        printf("Failed to create commited resource for default buffer.\n");
        return result;
    }

    //Creating GPU upload buffer.
    CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES::CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    result = m_Device->CreateCommittedResource(
        &uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
        &defaultBufferResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&gpuUploadBuffer));

    if (FAILED(result))
    {
        printf("Failed to create commited resource for default buffer's GPU upload buffer.\n");
        return result;
    }

    //Describe data for copy into buffer.
    D3D12_SUBRESOURCE_DATA subresourceData{};
    subresourceData.pData = data;
    subresourceData.RowPitch = sizeBytes;
    subresourceData.SlicePitch = sizeBytes;

    //Scheduling copy to the default buffer.
    //Copies CPU memory to immediate upload heap, then upload heap is copied into buffer by gpu.
    CD3DX12_RESOURCE_BARRIER toCopyTransition = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
    m_CommandList->ResourceBarrier(1, &toCopyTransition);

    UpdateSubresources(m_CommandList,
        defaultBuffer, gpuUploadBuffer, 0,
        0, 1, &subresourceData);

    CD3DX12_RESOURCE_BARRIER toReadTransition = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ);
    m_CommandList->ResourceBarrier(1, &toReadTransition);

    result = ExecuteAndResetCommandList();

    if (FAILED(result))
    {
        printf("Failed to execute and reset command list during default buffer creation.\n");
        return result;
    }

    return S_OK;
}

HRESULT DX12Renderer::CreateInputLayout()
{
    Vertex::GetElementDescription(m_DefaultInputLayout);
    return S_OK;
}

void DX12Renderer::DestroyInputLayout()
{
    m_DefaultInputLayout.clear();
}

void DX12Renderer::UploadPushConstants()
{
    m_CommandList->SetGraphicsRoot32BitConstants(1, sizeof(PushConstants) / sizeof(UINT32), m_PushConstants, 0);
}

HRESULT DX12Renderer::UpdateWorldMatrix(const Matrix4x4& worldMatrix)
{
    assert(m_IsInitialised);
    m_PushConstants->World = worldMatrix;
    UploadPushConstants();
    return S_OK;
}

HRESULT DX12Renderer::UpdateLightingBuffer(const LightBuffer& lb)
{
    assert(m_IsInitialised);

    if (m_LightBufferAddressArray[m_CurrentBackbufferIndex] != nullptr)
    {
        m_CommandList->SetGraphicsRootConstantBufferView(2, m_LightBufferGPUUploaderArray[m_CurrentBackbufferIndex]->GetGPUVirtualAddress());
        memcpy(m_LightBufferAddressArray[m_CurrentBackbufferIndex], &lb, sizeof(LightBuffer));
        return S_OK;
    }

    return E_FAIL;
}

HRESULT DX12Renderer::UpdateConstantBuffer(const ConstantBuffer& cb)
{
    assert(m_IsInitialised);

    if (m_ConstantBufferAddressArray[m_CurrentBackbufferIndex] != nullptr)
    {
        m_CommandList->SetGraphicsRootConstantBufferView(0, m_ConstantBufferGPUUploaderArray[m_CurrentBackbufferIndex]->GetGPUVirtualAddress());
        memcpy(m_ConstantBufferAddressArray[m_CurrentBackbufferIndex], &cb, sizeof(ConstantBuffer));
        return S_OK;
    }

    return E_FAIL;
}

bool DX12Renderer::BindGenericBufferData(GenericBuffer& genericBuffer, const void* buffer, const size_t& bufferLength)
{
    ID3D12Resource* gpuUploader = nullptr;
    
    HRESULT hr = CreateDefaultBuffer(genericBuffer.m_Resource, gpuUploader, buffer, bufferLength);

    if (FAILED(hr) || genericBuffer.IsLoaded() == false)
    {
        printf("Failed to bind data to a buffer.");

        if (genericBuffer.m_Resource != nullptr)
        {
            genericBuffer.m_Resource->Release();
            genericBuffer.m_Resource = nullptr;
        }
        
        if (gpuUploader != nullptr)
        {
            gpuUploader->Release();
            gpuUploader = nullptr;
        }

        return false;
    }

    return true;
}

bool DX12Renderer::BindVertexData(VertexBuffer& vertexBuffer, const void* buffer, const size_t& bufferLength)
{
    bool boundDataAsGeneric = BindGenericBufferData(vertexBuffer, buffer, bufferLength);

    if (boundDataAsGeneric == false)
    {
        printf("Failed to bind vertex buffer data. Error during generic buffer binding stage.");
        return false;
    }

    CD3DX12_RESOURCE_BARRIER vbTransition = CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    GetCommandList()->ResourceBarrier(1, &vbTransition);

    vertexBuffer.m_VertexBufferView.BufferLocation = vertexBuffer.GetResource()->GetGPUVirtualAddress();
    vertexBuffer.m_VertexBufferView.SizeInBytes = (UINT)bufferLength;
    vertexBuffer.m_VertexBufferView.StrideInBytes = sizeof(Vertex);

    vertexBuffer.m_ElementCount = ((UINT)bufferLength / (UINT)sizeof(Vertex));

    return true;
}

bool DX12Renderer::BindIndexData(IndexBuffer& indexBuffer, const void* buffer, const size_t& bufferLength, const bool& isShortIndex)
{
    bool boundDataAsGeneric = BindGenericBufferData(indexBuffer, buffer, bufferLength);

    if (boundDataAsGeneric == false)
    {
        printf("Failed to bind index buffer data. Error during generic buffer binding stage.");
        return false;
    }

    CD3DX12_RESOURCE_BARRIER vbTransition = CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    GetCommandList()->ResourceBarrier(1, &vbTransition);

    indexBuffer.m_IndexBufferView.BufferLocation = indexBuffer.GetResource()->GetGPUVirtualAddress();
    indexBuffer.m_IndexBufferView.SizeInBytes = (UINT)bufferLength;
    indexBuffer.m_IndexBufferView.Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
    indexBuffer.m_ElementCount = ((UINT)bufferLength / (UINT)sizeof(unsigned int));

    if (isShortIndex)
    {
        indexBuffer.m_IndexBufferView.Format = DXGI_FORMAT::DXGI_FORMAT_R16_UINT;
        indexBuffer.m_ElementCount = ((UINT)bufferLength / (UINT)sizeof(unsigned short));
    }

    return true;
}

TextureRef DX12Renderer::BindTextureData(const int& indexToBindTo, const void* data, const size_t& len, const Vector2& dimensions)
{
    if (data == nullptr || len <= 0)
    {
        printf("Failed to bind texture. Invalid data or buffer length.\n");
        return nullptr;
    }

    int width = (int)dimensions.x;
    int height = (int)dimensions.y;

    if (width <= 0 || height <= 0)
    {
        printf("Failed to bind texture. Invalid texture size.\n");
        return nullptr;
    }

    HRESULT result = S_OK;

    ID3D12Resource* textureResource = nullptr;

    D3D12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, width, height);

    resourceDesc.MipLevels = static_cast<UINT16>(std::floor(std::log2(max(dimensions.x, dimensions.y)))) + 1;

    result = m_Device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&textureResource));

    if (FAILED(result))
    {
        printf("Failed to create commited resource for texture.\n");
        return nullptr;
    }

    ID3D12Resource* textureUploadHeap = nullptr;
    D3D12_HEAP_PROPERTIES cpuUploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD);
    UINT64 uploadSize = GetRequiredIntermediateSize(textureResource, 0, 1);

    CD3DX12_RESOURCE_DESC gpuUploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);

    result = m_Device->CreateCommittedResource(
        &cpuUploadHeap,
        D3D12_HEAP_FLAG_NONE,
        &gpuUploadBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureUploadHeap));

    if (FAILED(result))
    {
        printf("Failed to create commited GPU resources (upload heap) for texture.\n");
        return nullptr;
    }

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = data;
    textureData.RowPitch = (width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
    textureData.SlicePitch = textureData.RowPitch * height;

    UpdateSubresources(m_CommandList, textureResource, textureUploadHeap, 0, 0, 1, &textureData);

    CD3DX12_RESOURCE_BARRIER copyToSRVTransition = CD3DX12_RESOURCE_BARRIER::Transition(textureResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    m_CommandList->ResourceBarrier(1, &copyToSRVTransition);

    D3D12_RESOURCE_DESC textureResourceDesc = textureResource->GetDesc();

    CD3DX12_SHADER_RESOURCE_VIEW_DESC srvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
    srvDesc.Format = textureResourceDesc.Format;
    srvDesc.Texture2D.MipLevels = textureResourceDesc.MipLevels;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

    D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = {};

    DX12Renderer::GetNewDescriptorHandleFromMainSRVHeap(&srvCpuHandle, &srvGpuHandle);

    m_Device->CreateShaderResourceView(textureResource, &srvDesc, srvCpuHandle);

    result = ExecuteAndResetCommandList();

    if (FAILED(result))
    {
        printf("Failed to execute and reset command list during texture buffer creation.\n");
        return nullptr;
    }

    TextureRef texture = nullptr;

    if (textureResource != nullptr)
    {
        texture = std::make_shared<Texture>();
        texture->m_Height = height;
        texture->m_Width = width;
        texture->m_IsLoaded = true;
        texture->m_ID = indexToBindTo;
        texture->m_CPUHandle = srvCpuHandle;
        texture->m_GPUHandle = srvGpuHandle;
        texture->m_Resource = textureResource;
    }

    return texture;
}

void DX12Renderer::ClearFrame()
{
    assert(m_IsInitialised);

    if (FAILED(m_CommandAllocator->Reset()))
    {
        printf("Failed to reset command allocator.\n");
        return;
    }

    if (FAILED(ResetCommandList()))
    {
        printf("Failed to reset command allocator.\n");
        return;
    }

    m_CommandList->RSSetViewports(1, &m_Viewport);
    m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

    D3D12_CPU_DESCRIPTOR_HANDLE backBufferHandle = GetCurrentBackbufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvBufferHandle = GetDepthStencilBufferView();

    D3D12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    m_CommandList->ResourceBarrier(1, &transition);

    float Colour[4] = { m_ClearColour.x, m_ClearColour.y, m_ClearColour.z, m_ClearColour.w };
    m_CommandList->ClearDepthStencilView(dsvBufferHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    transition = CD3DX12_RESOURCE_BARRIER::Transition(m_SwapchainBuffers[m_CurrentBackbufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_CommandList->ResourceBarrier(1, &transition);

    m_CommandList->ClearRenderTargetView(backBufferHandle, Colour, 0, nullptr);
    m_CommandList->OMSetRenderTargets(1, &backBufferHandle, true, &dsvBufferHandle);
}

void DX12Renderer::PresentFrame()
{
    assert(m_IsInitialised);

    D3D12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition(m_SwapchainBuffers[m_CurrentBackbufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_CommandList->ResourceBarrier(1, &transition);

    transition = CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PRESENT);
    m_CommandList->ResourceBarrier(1, &transition);

    if (FAILED(m_CommandList->Close()))
    {
        printf("Failed to close command list.\n");
        return;
    }

    ID3D12CommandList* commandLists = { m_CommandList };
    m_CommandQueue->ExecuteCommandLists(1, &commandLists);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        // Update and Render additional Platform Windows
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    HRESULT hr = m_SwapChain->Present(0, 0);

    if (FAILED(hr))
    {
        HRESULT removalReason = m_Device->GetDeviceRemovedReason();
        printf("Failed to present swap chain. Removal reason: %i\n", (int)removalReason);
        return;
    }

    m_CurrentBackbufferIndex = (m_CurrentBackbufferIndex + 1) % m_SwapChainBufferCount;
    if (FAILED(FlushCommandQueue()))
    {
        printf("Failed to flush command queue.\n");
        return;
    }
}
