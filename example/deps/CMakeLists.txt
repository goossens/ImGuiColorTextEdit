#	TextEditor - A syntax highlighting text editor for ImGui
#	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
#
#	This work is licensed under the terms of the MIT license.
#	For a copy, see <https://opensource.org/licenses/MIT>.

include(FetchContent)

if(NOT D3D11)
	add_subdirectory(sdl)
endif()

add_subdirectory(freetype)
add_subdirectory(imgui)
add_subdirectory(imguifiledialog)
add_subdirectory(lsp)
