#pragma once


//
//	TextEditorGcode
//


struct TextEditorGcode {
	static TextEditor::Iterator skipPosInteger(TextEditor::Iterator i, TextEditor::Iterator end)
	{
		while ((i < end) && (*i >= '0') && (*i <= '9')) {
			++i;
		}

		return i;
	}


	static TextEditor::Iterator skipInteger(TextEditor::Iterator i, TextEditor::Iterator end)
	{
		if (i < end) {
			if (*i == '-') {
				++i;
			}

			i = skipPosInteger(i, end);
		}

		return i;
	}


	static TextEditor::Iterator skipReal(TextEditor::Iterator i, TextEditor::Iterator end)
	{
		i = skipInteger(i, end);
		
		if (i < end) {
			if (*i == '.') {
				++i;
			}

			i = skipPosInteger(i, end);
		}

		return i;
	}


	static TextEditor::Iterator tokenize(TextEditor::Iterator start, TextEditor::Iterator end, TextEditor::Color& color)
	{
		const TextEditor::Color colorGcode = TextEditor::Color::keyword;
		const TextEditor::Color colorMcode = TextEditor::Color::keyword;
		const TextEditor::Color colorXIcode = TextEditor::Color::customColor1;
		const TextEditor::Color colorYJcode = TextEditor::Color::customColor4;
		const TextEditor::Color colorZKcode = TextEditor::Color::customColor7;
		const TextEditor::Color colorTFScode = TextEditor::Color::customColor5;
		const TextEditor::Color colorOther = TextEditor::Color::customColor9;

		TextEditor::Iterator i = start;
		bool integerOnly = false;

		if (i >= end) {
			return end;
		}

		switch (*i) {
			case 'G':
				color = colorGcode;
				integerOnly = true;
				break;

			case 'M':
				color = colorMcode;
				integerOnly = true;
				break;

			case 'T':
				color = colorTFScode;
				integerOnly = true;
				break;

			case 'F': case 'S':
				color = colorTFScode;
				break;

			case 'X': case 'I': case 'U':
				color = colorXIcode;
				break;

			case 'Y': case 'J': case 'V':
				color = colorYJcode;
				break;

			case 'Z': case 'K': case 'W':
				color = colorZKcode;
				break;

			default:
				color = colorOther;
				break;
		}

		++i;

		if (integerOnly) {
			i = skipInteger(i, end);
		}
		else {
			i = skipReal(i, end);
		}
	
		return i;
	}


	static const TextEditor::Language* getDefinition() {
		static bool initialized = false;
		static TextEditor::Language language;

		if (!initialized) {
			language.name = "Gcode";
			language.commentStart = "(";
			language.commentEnd = ")";
			language.singleLineComment = ";";

			language.customTokenizer = tokenize;
			initialized = true;
		}

		return &language;
	}
};


