#include "RenderCanvas.h"

ID3D11Device* device = nullptr;
ID3D11DeviceContext* immediateContext = nullptr;
ID3D11RenderTargetView* renderTargetView = nullptr;

HRESULT(*presenth)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) = nullptr;
HRESULT(*resizeh)(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) = nullptr;

WNDPROC oriWndProc = NULL;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int Screen_X, Screen_Y;

HWND hwnd = NULL;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) && settings.menu)
	{
		return true;
	}
	return CallWindowProc(oriWndProc, hWnd, msg, wParam, lParam);
}

HRESULT present_hk(IDXGISwapChain* swapchain, UINT sync, UINT flags)
{
	if (!device)
	{
		ID3D11Texture2D* renderTarget = 0;
		ID3D11Texture2D* backBuffer = 0;
		D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
		swapchain->GetDevice(__uuidof(device), (PVOID*)&device);
		device->GetImmediateContext(&immediateContext);

		swapchain->GetBuffer(0, __uuidof(renderTarget), (PVOID*)&renderTarget);
		device->CreateRenderTargetView(renderTarget, nullptr, &renderTargetView);
		renderTarget->Release();
		swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (PVOID*)&backBuffer);
		backBuffer->GetDesc(&backBufferDesc);

		Screen_X = backBufferDesc.Width;
		Screen_Y = backBufferDesc.Height;

		backBuffer->Release();
		if (!hwnd)
		{
			hwnd = FindWindowW(L"UnrealWindow", L"Fortnite  ");

			if (!hwnd)
				hwnd = GetForegroundWindow();
		}

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		X = (float)backBufferDesc.Width;
		Y = (float)backBufferDesc.Height;

		// Render Fonts

		ImGui_ImplDX11_Init(hwnd, device, immediateContext);
		ImGui_ImplDX11_CreateDeviceObjects();
	}
	immediateContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
	auto& window = Utils::createscene();

	// Render EVERYTHING here
	fn::actorloop(window);
	fn::menu();
	fn::render(window);
	// end rendering 

	Utils::destroyscene(window); // end scene
	return presenth(swapchain, sync, flags);
}

HRESULT resize_hk(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
	ImGui_ImplDX11_Shutdown();
	renderTargetView->Release();
	immediateContext->Release();
	device->Release();
	device = nullptr;
	return resizeh(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
}

ImGuiWindow& Utils::createscene() {
	ImGui_ImplDX11_NewFrame();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	ImGui::Begin("##createscene", nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);
	auto& io = ImGui::GetIO();
	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);
	return *ImGui::GetCurrentWindow();
}

VOID Utils::destroyscene(ImGuiWindow& window) {
	window.DrawList->PushClipRectFullScreen();
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
	ImGui::Render();
}