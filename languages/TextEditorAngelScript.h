//	TextEditor - A syntax highlighting text editor for Dear ImGui.
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	GetLanguageDefinitionAngelScript
//

static const TextEditor::Language* GetLanguageDefinitionAngelScript() {
	static bool initialized = false;
	static TextEditor::Language language;

	if (!initialized) {
		language.name = "AngelScript";
		language.preprocess = '#';
		language.singleLineComment = "//";
		language.commentStart = "/*";
		language.commentEnd = "*/";
		language.hasSingleQuotedStrings = true;
		language.hasDoubleQuotedStrings = true;
		language.stringEscape = '\\';

		static const char* const keywords[] = {
			"and", "abstract", "auto", "bool", "break", "case", "cast", "class", "const", "continue", "default",
			"do", "double", "else", "enum", "false", "final", "float", "for", "from", "funcdef", "function", "get",
			"if", "import", "in", "inout", "int", "interface", "int8", "int16", "int32", "int64", "is", "mixin",
			"namespace", "not", "null", "or", "out", "override", "private", "protected", "return", "set", "shared",
			"super", "switch", "this ", "true", "typedef", "uint", "uint8", "uint16", "uint32", "uint64", "void",
			"while", "xor"
		};

		for (auto& keyword : keywords) { language.keywords.insert(keyword); }

		language.isPunctuation = isCStylePunctuation;
		language.getIdentifier = getCStyleIdentifier;
		language.getNumber = getCStyleNumber;
		initialized = true;
	}

	return &language;
}
