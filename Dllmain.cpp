#include "RenderHelper.h"


void HOOKInitalize()
{
	L::MainGay();
	HWND window = FindWindow(0, (L"Fortnite  "));

	IDXGISwapChain* swapChain = nullptr;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;
	auto                 featureLevel = D3D_FEATURE_LEVEL_11_0;

	DXGI_SWAP_CHAIN_DESC sd = { 0 };
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.OutputWindow = window;
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;

	if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, 0, &featureLevel, 1, D3D11_SDK_VERSION, &sd, &swapChain, &device, nullptr, &context))) {
		MessageBox(0, (L"dx11 fatal error"), (L"fatal error"), MB_ICONERROR);
		return;
	}

	auto table = *reinterpret_cast<PVOID**>(swapChain);
	auto present = table[8];
	auto resize = table[13];

	context->Release();
	device->Release();
	swapChain->Release();

	MH_Initialize();
	auto addr = SDK::Utilities::Scanners::PatternScan(xorthis("48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 B8 0F 29 78 A8 44 0F 29 40 ? 44 0F 29 48 ? 44 0F 29 90 ? ? ? ? 44 0F 29 98 ? ? ? ? 44 0F 29 A0 ? ? ? ? 44 0F 29 A8 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B F1"));
	MH_CreateHook((LPVOID)addr, (LPVOID)CalculateShotHook, (LPVOID*)&CalculateShot);
	MH_EnableHook((LPVOID)addr);

	MH_CreateHook(present, present_hk, reinterpret_cast<PVOID*>(&presenth));
	MH_EnableHook(present);

	MH_CreateHook(resize, resize_hk, reinterpret_cast<PVOID*>(&resizeh));
	MH_EnableHook(resize);

	oriWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
}

HANDLE(WINAPI* Real_CreateFileW) (LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) = CreateFileW;
BOOL(WINAPI* Real_CreateDirectoryW) (LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) = CreateDirectoryW;

HANDLE WINAPI _CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{

	if (wcsstr(lpFileName, L".pak") || wcsstr(lpFileName, L".sig") || wcsstr(lpFileName, L"Fortnite") || wcsstr(lpFileName, L"\\.\\"))
		return Real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	else
		return Real_CreateFileW(L"C:\\Windows\\a", dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

BOOL WINAPI _CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	if (wcsstr(lpPathName, L"Fortnite"))
		return Real_CreateDirectoryW(lpPathName, lpSecurityAttributes);
	else
		return Real_CreateDirectoryW(L"C:\\Windows\\a", lpSecurityAttributes);
}

VOID InitAntiTracesFiles()
{
	DetourTransactionBegin();
	DetourAttach(&(PVOID&)Real_CreateFileW, _CreateFileW);
	DetourAttach(&(PVOID&)Real_CreateDirectoryW, _CreateDirectoryW);
}

void CheatInitialize()
{
	InitAntiTracesFiles();
	HOOKInitalize();
	Details.UWORLD = SDK::Utilities::Scanners::PatternScan(xorthis("48 8B 05 ? ? ? ? 4D 8B C2"));
	Details.UWORLD = RELATIVE_ADDR(Details.UWORLD, 7);
	LineOfS = SDK::Utilities::Scanners::PatternScan(xorthis("E8 ? ? ? ? 48 8B 0D ? ? ? ? 33 D2 40 8A F8"));
	LineOfS = RELATIVE_ADDR(LineOfS, 5);
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		//HOOKInitalize();
		CheatInitialize();
	}

	return TRUE;
}
