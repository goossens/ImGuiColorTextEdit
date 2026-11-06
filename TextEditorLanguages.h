//	TextEditor - A syntax highlighting text editor for Dear ImGui.
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	This file configures the language definitions. By default, all provided languages are included.
//	If you want to save space, you can deactivate/remove unwanted/unused definitions from this file
//	by commenting out or removing the corresponding include and RegisterLanguage statements.
//
//	One important note is that some languages reuse parts of the C language definition so although you
//	might not need C directly, if you use AngelScript, C++, C#, GLSL, HLSL, Lua, Python, SQL or Wgsl, you
//	must also include the C definition to avoid link errors.
//
//	If you want to add your own languages, you must create new language definition files in a place where
//	the compiler can find them and add an include and RegisterLanguage statement below below for each
//	language. Any of the language definitions in the languages folder can be used as a template.


// don't remove this if you include AngelScript, C++, C#, GLSL, HLSL, Lua, Python, SQL or Wgsl
#include "languages/TextEditorC.h"

// these are all optional
#include "languages/TextEditorAngelScript.h"
#include "languages/TextEditorCpp.h"
#include "languages/TextEditorCs.h"
#include "languages/TextEditorGlsl.h"
#include "languages/TextEditorHlsl.h"
#include "languages/TextEditorJson.h"
#include "languages/TextEditorJulia.h"
#include "languages/TextEditorLua.h"
#include "languages/TextEditorMarkdown.h"
#include "languages/TextEditorPython.h"
#include "languages/TextEditorSql.h"
#include "languages/TextEditorWgsl.h"


//
//	TextEditor::registerLanguages
//

void TextEditor::registerLanguages() {
	// don't remove this if you include AngelScript, C++, C#, GLSL, HLSL, Lua, Python, SQL or Wgsl
	TextEditor::RegisterLanguage("C", GetLanguageDefinitionC, {".c"});

	// these are all optional
	TextEditor::RegisterLanguage("AngelScript", GetLanguageDefinitionAngelScript, {".as"});
	TextEditor::RegisterLanguage("C++", GetLanguageDefinitionCpp, {".cpp", ".h", ".hpp"});
	TextEditor::RegisterLanguage("C#", GetLanguageDefinitionCs, {".cs"});
	TextEditor::RegisterLanguage("GLSL", GetLanguageDefinitionGlsl, {".glsl"});
	TextEditor::RegisterLanguage("HLSL", GetLanguageDefinitionHlsl, {".hlsl"});
	TextEditor::RegisterLanguage("JSON", GetLanguageDefinitionJson, {".json"});
	TextEditor::RegisterLanguage("Julia", GetLanguageDefinitionJulia, {".jl"});
	TextEditor::RegisterLanguage("Lua", GetLanguageDefinitionLua, {".lua"});
	TextEditor::RegisterLanguage("Markdown", GetLanguageDefinitionMarkdown, {".md"});
	TextEditor::RegisterLanguage("Python", GetLanguageDefinitionPython, {".py"});
	TextEditor::RegisterLanguage("SQL", GetLanguageDefinitionSql, {".sql"});
	TextEditor::RegisterLanguage("WGSL", GetLanguageDefinitionWgsl, {".wgsl"});
}

