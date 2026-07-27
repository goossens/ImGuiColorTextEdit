//	TextEditor - A syntax highlighting text editor for ImGui
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#pragma comment(lib, "d3d11.lib")

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#undef APIENTRY

#include <algorithm>
#include <format>
#include <string>

#include <d3d11.h>
#include <tchar.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "editor.h"
#include "dejavu.h"


//
//	data
//

static ID3D11Device* d3dDevice = nullptr;
static ID3D11DeviceContext* d3dDeviceContext = nullptr;
static IDXGISwapChain* swapChain = nullptr;
static bool swapChainOccluded = false;
static UINT resizeWidth = 0;
static UINT resizeHeight = 0;
static ID3D11RenderTargetView* mainRenderTargetView = nullptr;


//
//	forward declarations
//

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


//
//	example
//

int example() {
	// make process DPI aware and obtain main monitor scale
	ImGui_ImplWin32_EnableDpiAwareness();
	float mainScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));

	// create application window
	WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
	::RegisterClassExW(&wc);
	HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"TextEditor Example", WS_OVERLAPPEDWINDOW, 100, 100, (int)(1280 * mainScale), (int)(800 * mainScale), nullptr, nullptr, wc.hInstance, nullptr);

	// initialize Direct3D
	if (!CreateDeviceD3D(hwnd)) {
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	ImGui::StyleColorsDark();

	// setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(mainScale);
	style.FontScaleDpi = mainScale;

	// setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(d3dDevice, d3dDeviceContext);

	// setup our font
	ImFontConfig config;
	std::copy_n("DejaVu", 7, config.Name);
	config.FontDataOwnedByAtlas = false;
	io.Fonts->Clear();
	io.Fonts->AddFontFromMemoryCompressedTTF(static_cast<const void*>(&dejavu), dejavuSize, 15.0f, &config);

	// main loop
	Editor editor;
	MSG msg;

	// generate debug information
	editor.setDebugInformation([&]() {
		std::string information = "Backend: DirectX11\n";
		RECT rect;
		::GetWindowRect(hwnd, &rect);
		information += std::format("GetWindowRect: {}, {}\n", rect.right - rect.left, rect.bottom - rect.top);
		information += std::format("GetDpiScaleForMonitor: {}\n", mainScale);
		return information;
	});

	while (!editor.isDone()) {
		// poll and handle messages (inputs, window resize, etc.)
		// See the WndProc() function below for our to dispatch events to the Win32 backend.
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				editor.tryToQuit();
			}
		}

		// handle window being minimized or screen locked
		if (swapChainOccluded && swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
			::Sleep(10);
			continue;
		}

		swapChainOccluded = false;

		// handle window resize (we don't resize directly in the WM_SIZE handler)
		if (resizeWidth != 0 && resizeHeight != 0) {
			CleanupRenderTarget();
			swapChain->ResizeBuffers(0, resizeWidth, resizeHeight, DXGI_FORMAT_UNKNOWN, 0);
			resizeWidth = resizeHeight = 0;
			CreateRenderTarget();
		}

		// start a Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// render editor
		editor.render();

		// rendering
		ImGui::Render();
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		d3dDeviceContext->OMSetRenderTargets(1, &mainRenderTargetView, nullptr);
		d3dDeviceContext->ClearRenderTargetView(mainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// present to screen
		HRESULT hr = swapChain->Present(1, 0);   // Present with vsync
		swapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
	}

	// cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);

	return 0;
}


//
// Helper functions
//

bool CreateDeviceD3D(HWND hWnd) {
	// setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &swapChain, &d3dDevice, &featureLevel, &d3dDeviceContext);

	if (res == DXGI_ERROR_UNSUPPORTED) {
		// try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &swapChain, &d3dDevice, &featureLevel, &d3dDeviceContext);
	}

	if (res != S_OK) {
		return false;
	}

	CreateRenderTarget();
	return true;
}


void CleanupDeviceD3D() {
	CleanupRenderTarget();
	if (swapChain) { swapChain->Release(); swapChain = nullptr; }
	if (d3dDeviceContext) { d3dDeviceContext->Release(); d3dDeviceContext = nullptr; }
	if (d3dDevice) { d3dDevice->Release(); d3dDevice = nullptr; }
}


void CreateRenderTarget() {
	ID3D11Texture2D* pBackBuffer;
	swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
	pBackBuffer->Release();
}


void CleanupRenderTarget() {
	if (mainRenderTargetView) { mainRenderTargetView->Release(); mainRenderTargetView = nullptr; }
}


// forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


//
//	Win32 message handler
//

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg) {
		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED) {
				return 0;
			}

			resizeWidth = (UINT)LOWORD(lParam); // Queue resize
			resizeHeight = (UINT)HIWORD(lParam);
			return 0;

		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_KEYMENU) {
				// Disable ALT application menu
				return 0;
			}

			break;

		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
	}

	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
