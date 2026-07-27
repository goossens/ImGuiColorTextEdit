//	TextEditor - A syntax highlighting text editor for ImGui
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <algorithm>
#include <format>
#include <string>

#include <SDL3/SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

#include "editor.h"
#include "dejavu.h"


//
//	example
//

int example() {
	// setup SDL
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error: SDL_Init(): %s\n", SDL_GetError());
		return -1;
	}

	// create SDL window graphics context
	SDL_WindowFlags windowFlags =
		SDL_WINDOW_RESIZABLE |
		SDL_WINDOW_HIGH_PIXEL_DENSITY;

	float mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

	SDL_Window* window = SDL_CreateWindow(
		"TextEditor Example",
		static_cast<int>(1280 * mainScale),
		static_cast<int>(720 * mainScale),
		windowFlags);

	if (window == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		return -1;
	}

	// create GPU device
	SDL_GPUShaderFormat formatFlags =
		SDL_GPU_SHADERFORMAT_SPIRV |
		SDL_GPU_SHADERFORMAT_DXBC |
		SDL_GPU_SHADERFORMAT_MSL |
		SDL_GPU_SHADERFORMAT_METALLIB;

	SDL_GPUDevice* gpu_device = SDL_CreateGPUDevice(formatFlags, true, nullptr);

	if (gpu_device == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error: SDL_CreateGPUDevice(): %s\n", SDL_GetError());
		return -1;
	}

	// claim window for GPU device
	if (!SDL_ClaimWindowForGPUDevice(gpu_device, window)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error: SDL_ClaimWindowForGPUDevice(): %s\n", SDL_GetError());
		return -1;
	}

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

	// setup platform/renderer backend
	ImGui_ImplSDL3_InitForSDLGPU(window);
	ImGui_ImplSDLGPU3_InitInfo initInfo = {};
	initInfo.Device = gpu_device;
	initInfo.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu_device, window);
	initInfo.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
	ImGui_ImplSDLGPU3_Init(&initInfo);

	// setup our font
	ImFontConfig config;
	std::copy_n("DejaVu", 7, config.Name);
	config.FontDataOwnedByAtlas = false;
	io.Fonts->Clear();
	io.Fonts->AddFontFromMemoryCompressedTTF(static_cast<const void*>(&dejavu), dejavuSize, 15.0f, &config);

	// main loop
	Editor editor;
	SDL_Event event;

	// generate debug information
	editor.setDebugInformation([&]() {
		std::string information = "Backend: SDL3GPU\n";
		int w, h;

		SDL_GetWindowSize(window, &w, &h);
		information += std::format("SDL_GetWindowSize: {}, {}\n", w, h);
		SDL_GetWindowSizeInPixels(window, &w, &h);
		information += std::format("SDL_GetWindowSizeInPixels: {}, {}\n", w, h);

		float scale;
		scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
		information += std::format("SDL_GetDisplayContentScale: {}\n", scale);
		scale = SDL_GetWindowDisplayScale(window);
		information += std::format("SDL_GetWindowDisplayScale: {}\n", scale);
		scale = SDL_GetWindowPixelDensity(window);
		information += std::format("SDL_GetWindowPixelDensity: {}\n\n", scale);
		return information;
	});

	while (!editor.isDone()) {
		// poll and handle events (inputs, window resize, etc.)
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL3_ProcessEvent(&event);

			if (event.type == SDL_EVENT_QUIT) {
				editor.tryToQuit();

			} else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window)) {
				editor.tryToQuit();
			}
		}

		if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
			SDL_Delay(10);
			continue;
		}

		// start a Dear ImGui frame
		ImGui_ImplSDLGPU3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		// render editor
		editor.render();

		// render to the screen
		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);

		// acquire a GPU command buffer and swapchain texture
		SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(gpu_device);
		SDL_GPUTexture* swapchain_texture;
		SDL_AcquireGPUSwapchainTexture(command_buffer, window, &swapchain_texture, nullptr, nullptr);

		if (swapchain_texture != nullptr && !is_minimized) {
			// setup and start a render pass
			ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);

			SDL_GPUColorTargetInfo target_info = {};
			target_info.texture = swapchain_texture;
			target_info.load_op = SDL_GPU_LOADOP_CLEAR;
			target_info.store_op = SDL_GPU_STOREOP_STORE;
			target_info.mip_level = 0;
			target_info.layer_or_depth_plane = 0;
			target_info.cycle = false;
			SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, nullptr);

			// render ImGui
			ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, render_pass);
			SDL_EndGPURenderPass(render_pass);
		}

		// submit the command buffer
		SDL_SubmitGPUCommandBuffer(command_buffer);
	}

	// cleanup
	SDL_WaitForGPUIdle(gpu_device);
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplSDLGPU3_Shutdown();
	ImGui::DestroyContext();

	SDL_ReleaseWindowFromGPUDevice(gpu_device, window);
	SDL_DestroyGPUDevice(gpu_device);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
