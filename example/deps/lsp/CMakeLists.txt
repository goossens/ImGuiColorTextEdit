#	TextEditor - A syntax highlighting text editor for ImGui
#	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
#
#	This work is licensed under the terms of the MIT license.
#	For a copy, see <https://opensource.org/licenses/MIT>.

set(LSP_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Populate(lsp QUIET GIT_REPOSITORY "https://github.com/leon-bckl/lsp-framework" GIT_TAG "master")
add_subdirectory(${lsp_SOURCE_DIR} ${lsp_BINARY_DIR} EXCLUDE_FROM_ALL)
