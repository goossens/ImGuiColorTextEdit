//	TextEditor - A syntax highlighting text editor for Dear ImGui.
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	GetLanguageDefinitionCpp
//

static const TextEditor::Language* GetLanguageDefinitionCpp() {
	static bool initialized = false;
	static TextEditor::Language language;

	if (!initialized) {
		language.name = "C++";
		language.preprocess = '#';
		language.singleLineComment = "//";
		language.commentStart = "/*";
		language.commentEnd = "*/";
		language.hasSingleQuotedStrings = true;
		language.hasDoubleQuotedStrings = true;
		language.stringEscape = '\\';

		static const char* const keywords[] = {
			"alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit", "atomic_noexcept",
			"bitand", "bitor", "break", "case", "catch", "compl", "const_cast", "continue", "default", "delete",
			"do", "dynamic_cast", "else", "explicit", "export", "extern", "false", "for", "goto", "if", "import",
			"new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "reinterpret_cast", "requires",
			"return", "sizeof", "static_assert", "static_cast", "switch", "synchronized", "this", "thread_local",
			"throw", "true", "try", "while", "xor", "xor_eq"
		};

		static const char* const declarations[] = {
			"auto", "bool", "char", "char8_t", "char16_t", "char32_t", "class", "concept", "const", "constexpr", "decltype",
			"double", "explicit", "export", "extern", "enum", "extern", "float", "friend", "inline", "int", "long",
			"module", "mutable", "namespace", "private", "protected", "public", "register", "restrict", "short",
			"signed", "static", "struct", "template", "typedef", "typeid", "typename", "union", "using", "unsigned",
			"virtual", "void", "volatile", "wchar_t"
		};

		for (auto& keyword : keywords) { language.keywords.insert(keyword); }
		for (auto& declaration : declarations) { language.declarations.insert(declaration); }

		language.isPunctuation = isCStylePunctuation;
		language.getIdentifier = getCStyleIdentifier;
		language.getNumber = getCStyleNumber;
		initialized = true;
	}

	return &language;
}
