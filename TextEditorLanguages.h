//	TextEditor - A syntax highlighting text editor for Dear ImGui.
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//	This file configures the language definitions. By default, all provided languages are included.
//	If you want to save space, you can deactivate/remove unwanted/unused definitions from this file
//	by commenting out or removing the corresponding include and RegisterLanguage statements.
//
//	One important note is that some languages reuse parts of the C language definition so although you
//	might not need C directly, if you use AngelScript, C++, C#, GLSL, HLSL, Lua, Python, SQL or Wgsl,
//	the C definition is automatically included.
//
//	If you want to add your own languages, you must create new language definition files in a place where
//	the compiler can find them and add an include and RegisterLanguage statement below below for each
//	language. Any of the language definitions in the languages folder can be used as a template.

#include "languages/TextEditorAngelScript.h"
#include "languages/TextEditorC.h"
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
//	TextEditorLanguages
//

struct TextEditorLanguages {
	static void registerLanguages() {
		TextEditor::RegisterLanguage("AngelScript", TextEditorAngelScript::getDefinition, {".as"});
		TextEditor::RegisterLanguage("C", TextEditorC::getDefinition, {".c"});
		TextEditor::RegisterLanguage("C++", TextEditorCpp::getDefinition, {".cpp", ".h", ".hpp"});
		TextEditor::RegisterLanguage("C#", TextEditorCs::getDefinition, {".cs"});
		TextEditor::RegisterLanguage("GLSL", TextEditorGlsl::getDefinition, {".glsl"});
		TextEditor::RegisterLanguage("HLSL", TextEditorHlsl::getDefinition, {".hlsl"});
		TextEditor::RegisterLanguage("JSON", TextEditorJson::getDefinition, {".json"});
		TextEditor::RegisterLanguage("Julia", TextEditorJulia::getDefinition, {".jl"});
		TextEditor::RegisterLanguage("Lua", TextEditorLua::getDefinition, {".lua"});
		TextEditor::RegisterLanguage("Markdown", TextEditorMarkdown::getDefinition, {".md"});
		TextEditor::RegisterLanguage("Python", TextEditorPython::getDefinition, {".py"});
		TextEditor::RegisterLanguage("SQL", TextEditorSql::getDefinition, {".sql"});
		TextEditor::RegisterLanguage("WGSL", TextEditorWgsl::getDefinition, {".wgsl"});
	}
};
