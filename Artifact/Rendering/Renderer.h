#pragma once

class Texture;
struct GraphicsDevice;

class Renderer
{
private:

protected:
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

	bool CreateTexture(Texture& texture, const std::string& textureLocation);
	bool DestroyTexture(Texture& texture);

public:
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

	virtual bool ResizeSwapchain(const int& newWidth, const int& newHeight) = 0;

	virtual void ClearFrame() = 0;
	virtual void PresentFrame() = 0;
};

