#	ObjectTalk Scripting Language
#	Copyright (c) 1993-2026 Johan A. Goossens. All rights reserved.
#
#	This work is licensed under the terms of the MIT license.
#	For a copy, see <https://opensource.org/licenses/MIT>.

FetchContent_Populate(imguitoastr QUIET GIT_REPOSITORY "https://github.com/goossens/ImGuiToastr" GIT_TAG "main")

set(IMGUITOASTR_SOURCES
	${imguitoastr_SOURCE_DIR}/Toastr.h
	${imguitoastr_SOURCE_DIR}/Toastr.cpp)

add_library(imguitoastr STATIC ${IMGUITOASTR_SOURCES})
target_include_directories(imguitoastr PUBLIC ${imguitoastr_SOURCE_DIR})
target_link_libraries(imguitoastr imgui)

set_target_properties(imguitoastr PROPERTIES FOLDER deps)
