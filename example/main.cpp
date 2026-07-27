//	TextEditor - A syntax highlighting text editor for ImGui
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Configure platform
//
//	Default backend is the SDL3 GPU backend
//	Other backends can be specified in CMakeLists.txt
//

#if D3D11
#include "wind3d11.h"

#else
#include "sdl3gpu.h"

#endif


//
//	main
//

int main(int, char**) {
	// the real main function is called example and
	// is located in the backend includes listed above
	return example();
}
