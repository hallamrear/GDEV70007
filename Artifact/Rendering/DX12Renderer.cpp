#include "pch.h"
#include "DX12Renderer.h"
#include <Rendering/ConstantBuffer.h>
#include <Rendering/Vertex.h>

#define FAILED_RETURN(hr) if(FAILED(hr)) return !FAILED(hr);

DX12Renderer::DX12Renderer() : Renderer()
{
    m_CBVHeap = nullptr;
    m_ClearColour = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    m_DXGIFactory = nullptr;
    m_Device = nullptr;
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
    m_WindowHeight = 0;
    m_WindowWidth = 0;
    m_SwapchainBuffers = nullptr;
    m_Viewport = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_ScissorRect = {};
    m_DepthStencilBuffer = nullptr;
    m_DefaultPipeline = nullptr;
    m_DefaultPixelShaderBlob = nullptr;
    m_DefaultVertexShaderBlob = nullptr;
    m_RootSignature = nullptr;

    for (size_t i = 0; i < MAX_NUM_ENTITIES; i++)
    {
        m_ConstantBufferArray[i] = { nullptr };
    }
}

DX12Renderer::~DX12Renderer()
{
    assert(IsInitialised() == false);
}

const GraphicsDevice* DX12Renderer::GetDevice() const
{
    assert(IsInitialised());
    return m_Device;
}

GraphicsDevice* DX12Renderer::GetDevice()
{
    assert(IsInitialised());
    return m_Device;
}

HRESULT DX12Renderer::CreateResource(ID3D12Resource& resource, const D3D12_RESOURCE_DESC& resDesc)
{
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(resDesc);
    return E_NOTIMPL;
}

bool DX12Renderer::Initialise(HWND windowHandle)
{
    if (IsInitialised())
    {
        printf("Calling initialise on a renderer object that already exists.\n");
        return false;
    }

    if (windowHandle == NULL)
    {
        printf("Passing an invalid window handle.\n");
        return false;
    }

    m_WindowHandle = windowHandle;

    HRESULT hr = S_OK;
    m_IsInitialised = true;

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

    hr = SetupInitialViewportAndScissorRect();
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

    if (IsInitialised() == false)
    {
        m_WindowHandle = NULL;
        printf("Failed to initialise \n");
    }
    else
    {
        printf("Initialised renderer.");
        //Setting to closed as the first refernce to the command list will open it.
        if (m_CommandList)
        {
            m_CommandList->Close();
        }
    }

    return m_IsInitialised;
}

bool DX12Renderer::Shutdown()
{
    if (!IsInitialised())
    {
        printf("Calling shutdown on a renderer object that doesn't exist.");
        return false;
    }

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
    m_IsInitialised = false;

    return true;
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

    //Create hardware device
    result = D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&m_Device));
    if (FAILED(result))
    {
        printf("Failed to create D3D12 Device.\n");
        return result;
    }

    return result;
}

void DX12Renderer::DestroyDeviceAndFactory()
{
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

    if (FAILED(result))
    {
        printf("Failed to create fence from ID3D12Device.\n");
        return result;
    }

    //Cache descriptor sizes for later.
    m_RTVDescriptorHeapSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_DSVDescriptorHeapSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_CBVSRVDescriptorHeapSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS multisampleQualityLevels;
    multisampleQualityLevels.Format = (DXGI_FORMAT)m_BackbufferFormat;
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

    result = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));

    if (FAILED(result))
    {
        printf("Failed to create command allocator from ID3D12Device.\n");
        return result;
    }

    result = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator, nullptr, IID_PPV_ARGS(&m_CommandList));

    if (FAILED(result))
    {
        printf("Failed to create command list from ID3D12Device.\n");
        return result;
    }

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

    DXGI_MODE_DESC bufferDesc{};
    bufferDesc.Width = 0;
    bufferDesc.Height = 0;
    bufferDesc.RefreshRate.Numerator = 60;
    bufferDesc.RefreshRate.Denominator = 1;
    bufferDesc.Format = m_BackbufferFormat;
    bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    RECT rect{};
    GetWindowRect(m_WindowHandle, &rect);
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
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

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
            printf("Failed to get swap chain buffer %i from swap chain.\n", (unsigned int)i);
            return result;
        }

        m_Device->CreateRenderTargetView(m_SwapchainBuffers[i], nullptr, rtvHandle);

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
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue;
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

    m_Device->CreateDepthStencilView(m_DepthStencilBuffer, nullptr, GetDepthStencilBufferView());

    D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    m_CommandList->ResourceBarrier(1, &resourceBarrier);

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

HRESULT DX12Renderer::SetupInitialViewportAndScissorRect()
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

    m_CommandList->RSSetViewports(1, &m_Viewport);

    m_ScissorRect.left = 0;
    m_ScissorRect.top = 0;
    m_ScissorRect.right = m_WindowWidth;
    m_ScissorRect.bottom = m_WindowHeight;
    m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

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
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantBuffer));
    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    for (uint32_t i = 0; i < MAX_NUM_ENTITIES; i++)
    {
        HRESULT result = m_Device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_ConstantBufferArray[i]));

        if (FAILED(result))
        {
            printf("Failed during constant buffer array %ui creation.\n", i);
            return result;
        }

        D3D12_GPU_VIRTUAL_ADDRESS bufferAddr = m_ConstantBufferArray[i]->GetGPUVirtualAddress();
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
        cbvDesc.BufferLocation = bufferAddr;
        cbvDesc.SizeInBytes = sizeof(ConstantBuffer);
        m_Device->CreateConstantBufferView(&cbvDesc, m_CBVHeap->GetCPUDescriptorHandleForHeapStart());
    }

    return S_OK;
}

void DX12Renderer::DestroyConstantBuffers()
{
    for (size_t i = 0; i < MAX_NUM_ENTITIES; i++)
    {
        if (m_ConstantBufferArray[i] != nullptr)
        {
            m_ConstantBufferArray[i]->Release();
            m_ConstantBufferArray[i] = nullptr;
        }
    }
}

HRESULT DX12Renderer::CreateConstantBufferHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
    heapDesc.NumDescriptors = 1;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;

    return m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_CBVHeap));
}

void DX12Renderer::DestroyConstantBufferHeap()
{
    if (m_CBVHeap != nullptr)
    {
        m_CBVHeap->Release();
        m_CBVHeap = nullptr;
    }
}

HRESULT DX12Renderer::CreateRootSignatureAndDescriptorTable()
{
    CD3DX12_ROOT_PARAMETER slotRootParameter[1]{};

    //Create a descriptor table for cbv
    CD3DX12_DESCRIPTOR_RANGE cbvTable{};
    cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

    slotRootParameter->InitAsDescriptorTable(1, &cbvTable);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(1, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

    m_CommandList->SetGraphicsRootSignature(m_RootSignature);
    ID3D12DescriptorHeap* heaps[] = { m_CBVHeap };

    m_CommandList->SetDescriptorHeaps(_countof(heaps), heaps);

    CD3DX12_GPU_DESCRIPTOR_HANDLE cbv(m_CBVHeap->GetGPUDescriptorHandleForHeapStart());
    cbv.Offset(0, m_CBVSRVDescriptorHeapSize);
    m_CommandList->SetGraphicsRootDescriptorTable(0, cbv);

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

    HRESULT result = E_POINTER;
    if (m_DefaultVertexShaderBlob != nullptr && m_DefaultPixelShaderBlob != nullptr)
    {
        pipelineStateDesc.InputLayout.NumElements = (uint32_t)m_DefaultInputLayout.size();
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
    }

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

HRESULT DX12Renderer::CreateInputLayout()
{
    Vertex::GetElementDescription(m_DefaultInputLayout);
    return S_OK;
}

void DX12Renderer::DestroyInputLayout()
{
    m_DefaultInputLayout.clear();
}

HRESULT DX12Renderer::UpdateConstantBuffer(const int& entityIndex, ConstantBuffer& cb)
{
    assert(m_IsInitialised);

    CD3DX12_GPU_DESCRIPTOR_HANDLE cbv(m_CBVHeap->GetGPUDescriptorHandleForHeapStart());
    cbv.Offset(entityIndex, m_CBVSRVDescriptorHeapSize);
    m_CommandList->SetGraphicsRootDescriptorTable(entityIndex, cbv);

    BYTE* mappedData = nullptr;
    HRESULT result = m_ConstantBufferArray[entityIndex]->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
    if (FAILED(result))
    {
        printf("Failed to map constant buffer address for update.\n");
        return result;
    }

    memcpy(&mappedData, &cb, sizeof(ConstantBuffer));

    if (m_ConstantBufferArray[entityIndex] != nullptr)
    {
        m_ConstantBufferArray[entityIndex]->Unmap(0, nullptr);
    }

    mappedData = nullptr;

    return S_OK;
}

void DX12Renderer::ClearFrame()
{
    assert(m_IsInitialised);

    if (FAILED(m_CommandAllocator->Reset()))
    {
        printf("Failed to reset command allocator.\n");
        return;
    }

    if (FAILED(m_CommandList->Reset(m_CommandAllocator, m_DefaultPipeline)))
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

    ID3D12DescriptorHeap* heaps[] = { m_CBVHeap };
    m_CommandList->SetGraphicsRootSignature(m_RootSignature);
    m_CommandList->SetDescriptorHeaps(_countof(heaps), heaps);
    CD3DX12_GPU_DESCRIPTOR_HANDLE cbv(m_CBVHeap->GetGPUDescriptorHandleForHeapStart());
    cbv.Offset(0, m_CBVSRVDescriptorHeapSize);
    m_CommandList->SetGraphicsRootDescriptorTable(0, cbv);
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
