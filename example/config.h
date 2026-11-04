//	TextEditor - A syntax highlighting text editor for ImGui
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once

// by default, the TextEditor includes all builtin languages and makes it a runtime choice to use them or not

// if you want to slim down the TextEditor, you can request just one or more language definitions to be included at compile time
// by uncommenting any of the INCLUDE statements below, you automatically override the default INCLUDE ALL behavior
// #define TEXTEDITOR_INCLUDE_ANGELSCRIPT
// #define TEXTEDITOR_INCLUDE_C
// #define TEXTEDITOR_INCLUDE_CPP
// #define TEXTEDITOR_INCLUDE_CS
// #define TEXTEDITOR_INCLUDE_GLSL
// #define TEXTEDITOR_INCLUDE_HLSL
// #define TEXTEDITOR_INCLUDE_JSON
// #define TEXTEDITOR_INCLUDE_LUA
// #define TEXTEDITOR_INCLUDE_MARKDOWN
// #define TEXTEDITOR_INCLUDE_PYTHON
// #define TEXTEDITOR_INCLUDE_SQL

// if you don't want any builtin language definitions, uncomment this
#define TEXTEDITOR_EXCLUDE_ALL

// if you just want to exclude some languages, uncomment sone of the following
// #define TEXTEDITOR_EXCLUDE_ANGELSCRIPT
// #define TEXTEDITOR_EXCLUDE_C
// #define TEXTEDITOR_EXCLUDE_CPP
// #define TEXTEDITOR_EXCLUDE_CS
// #define TEXTEDITOR_EXCLUDE_GLSL
// #define TEXTEDITOR_EXCLUDE_HLSL
// #define TEXTEDITOR_EXCLUDE_JSON
// #define TEXTEDITOR_EXCLUDE_LUA
// #define TEXTEDITOR_EXCLUDE_MARKDOWN
// #define TEXTEDITOR_EXCLUDE_PYTHON
// #define TEXTEDITOR_EXCLUDE_SQL
