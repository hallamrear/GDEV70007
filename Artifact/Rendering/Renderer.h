#pragma once
#include "Texturing/Texture.h"
#include <Rendering/Camera.h>
#include <Rendering/Geometry/Model.h>

class Model;
class GenericBuffer;
class VertexBuffer;
class IndexBuffer;
struct GraphicsDevice;

class Renderer
{
private:
	friend class Texture;

protected:
	Camera m_Camera;

	static const enum TEXTURE_FORMAT m_BackbufferFormat;
	static const enum TEXTURE_FORMAT m_DepthStencilBufferFormat;
	static const int m_SwapChainBufferCount;

	bool m_IsInitialised;
	int m_WindowWidth;
	int m_WindowHeight;
	HWND m_WindowHandle;
	Vector4 m_ClearColour;

	Renderer();
	virtual ~Renderer() = 0;

	virtual bool Initialise(HWND windowHandle) = 0;
	virtual bool InitialiseIMGUI() = 0;
	virtual bool Shutdown() = 0;
	virtual bool ShutdownIMGUI() = 0;

public:
	enum TEXTURE_TYPE_SLOT : int
	{
		TEXTURE_TYPE_DIFFUSE = 0,
		TEXTURE_TYPE_NORMAL = 1,
		TEXTURE_TYPE_COUNT = 2
	};

	const HWND& GetWindowHandle() const;
	HWND& GetWindowHandle();
	const int& GetWindowWidth() const;
	const int& GetWindowHeight() const;

	const bool& IsInitialised() const;

	static const TEXTURE_FORMAT GetBackbufferFormat();
	static const TEXTURE_FORMAT GetDepthStencilBufferFormat();
	static const int GetSwapChainBufferCount();

	static Renderer* CreateRenderer(HWND windowHandle);
	static bool DestroyRenderer(Renderer* renderer);

	virtual const GraphicsDevice* GetDevice() const = 0;
	virtual GraphicsDevice* GetDevice() = 0;

	const Vector4& GetClearColour() const;
	void SetClearColour(const Vector4& newColour);

	virtual bool BindVertexData(VertexBuffer& vertexBuffer, const void* buffer, const size_t& bufferLength) = 0;
	virtual bool BindIndexData(IndexBuffer& vertexBuffer, const void* buffer, const size_t& bufferLength, const bool& isShortIndex = false) = 0;
	virtual bool BindGenericBufferData(GenericBuffer& genericBuffer, const void* buffer, const size_t& bufferLength) = 0;

	virtual TextureRef BindTextureData(const int& indexToBindTo, const void* data, const size_t& len, const Vector2& dimensions) = 0;
	virtual	bool AssignTextureToTextureSlot(const TEXTURE_TYPE_SLOT& textureSlot, const TextureRef& texture) = 0;

	virtual bool ResizeSwapchain(const int& newWidth, const int& newHeight) = 0;

	virtual void PostAssetInitialisation() = 0;
	virtual void ClearFrame() = 0;
	virtual void PresentFrame() = 0;
	virtual void Render(const ModelRef& model, const Matrix4x4& worldMatrix) = 0;
	virtual void BeginIMGUIFrame() = 0;
	virtual void EndIMGUIFrame() = 0;

	virtual bool SetDebugDrawMode() = 0;
	virtual bool SetDefaultDrawMode() = 0;

	Camera& GetCamera();
};

