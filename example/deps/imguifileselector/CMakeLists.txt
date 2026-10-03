#	ObjectTalk Scripting Language
#	Copyright (c) 1993-2026 Johan A. Goossens. All rights reserved.
#
#	This work is licensed under the terms of the MIT license.
#	For a copy, see <https://opensource.org/licenses/MIT>.

FetchContent_Populate(imguifileselector QUIET GIT_REPOSITORY "https://github.com/goossens/ImGuiFileSelector" GIT_TAG "main")

set(IMGUITOASTR_SOURCES
	${imguifileselector_SOURCE_DIR}/FileSelector.h
	${imguifileselector_SOURCE_DIR}/FileSelector.cpp)

add_library(imguifileselector STATIC ${IMGUITOASTR_SOURCES})
target_include_directories(imguifileselector PUBLIC ${imguifileselector_SOURCE_DIR})
target_link_libraries(imguifileselector imgui)

set_target_properties(imguifileselector PROPERTIES FOLDER deps)
