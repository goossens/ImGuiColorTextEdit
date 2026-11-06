//	TextEditor - A syntax highlighting text editor for Dear ImGui.
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//	Contributed by James Wrigley (https://github.com/JamesWrigley)


//
//	isJuliaIdentifierStart
//

static bool isJuliaIdentifierStart(ImWchar c) {
	// see jl_id_start_char in Julia's src/flisp/julia_extensions.c
	// (currency symbols, other symbols and emoji are not supported as we don't have their Unicode categories)
#if defined(IMGUI_USE_WCHAR32)
	if (c >= 0x10000) {
		// variants of nabla and partial, bold and double-struck digits
		return TextEditor::CodePoint::isXidStart(c) ||
			c == 0x1d6c1 || c == 0x1d6db || c == 0x1d6fb || c == 0x1d715 || c == 0x1d735 ||
			c == 0x1d74f || c == 0x1d76f || c == 0x1d789 || c == 0x1d7a9 || c == 0x1d7c3 ||
			(c >= 0x1d7ce && c <= 0x1d7e1);
	}
#endif

	return TextEditor::CodePoint::isXidStart(c) ||
		// math symbols
		(c >= 0x2140 && c <= 0x2144) || c == 0x2200 || (c >= 0x2202 && c <= 0x2207) ||
		c == 0x220e || c == 0x220f || c == 0x2210 || c == 0x2211 ||
		c == 0x221e || c == 0x221f || (c >= 0x222b && c <= 0x2233) || c == 0x223f ||
		c == 0x22a4 || c == 0x22a5 || c == 0x22be || c == 0x22bf || (c >= 0x22c0 && c <= 0x22c3) ||
		(c >= 0x25f8 && c <= 0x25ff) || c == 0x266f || c == 0x27c0 || c == 0x27c1 || c == 0x27d8 || c == 0x27d9 ||
		(c >= 0x29b0 && c <= 0x29b4) || (c >= 0x2a00 && c <= 0x2a06) || (c >= 0x2a09 && c <= 0x2a16) ||
		c == 0x2a1b || c == 0x2a1c ||
		// super- and subscript +-=()
		(c >= 0x207a && c <= 0x207e) || (c >= 0x208a && c <= 0x208e) ||
		// angle symbols
		(c >= 0x2220 && c <= 0x2222) || (c >= 0x299b && c <= 0x29af) ||
		// katakana-hiragana sound marks (Other_ID_Start but not XID_Start)
		c == 0x309b || c == 0x309c;
}
//
//	isJuliaIdentifierChar
//

static bool isJuliaIdentifierChar(ImWchar c) {
	// see jl_id_char in Julia's src/flisp/julia_extensions.c (adds primes and super/subscripts)
	return TextEditor::CodePoint::isXidContinue(c) || isJuliaIdentifierStart(c) || c == '!' ||
		c == 0xb2 || c == 0xb3 || c == 0xb9 || (c >= 0x2032 && c <= 0x2037) || c == 0x2057 ||
		(c >= 0x2070 && c <= 0x209c) || (c >= 0x1d62 && c <= 0x1d6a);
}


//
//	getJuliaIdentifier
//

static TextEditor::Iterator getJuliaIdentifier(TextEditor::Iterator start, TextEditor::Iterator end) {
	TextEditor::Iterator i = start;

	if (i < end && isJuliaIdentifierStart(*i)) {
		i++;

		while (i < end && isJuliaIdentifierChar(*i)) {
			// a!=b is a != b
			if (*i == '!') {
				auto next = i;

				if (++next < end && *next == '=') {
					break;
				}
			}

			i++;
		}
	}

	return i;
}


//
//	followsJuliaOperand
//

static bool followsJuliaOperand(TextEditor::Iterator start) {
	// the previous character tells apart x' (adjoint) from 'x' (character) and 1:n (range) from :n (symbol)
	if (start.hasPrevious()) {
		const auto c = start.previous();
		return isJuliaIdentifierChar(c) || c == ')' || c == ']' || c == '}' || c == '\'' || c == '.';
	}

	return false;
}


//
//	isJuliaDigit
//

static bool isJuliaDigit(ImWchar c, int base) {
	if (base == 16) {
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');

	} else {
		return c >= '0' && c < static_cast<ImWchar>('0' + std::min(base, 10));
	}
}


//
//	skipJuliaDigits
//

static TextEditor::Iterator skipJuliaDigits(TextEditor::Iterator i, TextEditor::Iterator end, int base) {
	// underscores are allowed between digits
	while (i < end) {
		if (isJuliaDigit(*i, base)) {
			i++;

		} else if (*i == '_') {
			auto next = i;

			if (++next < end && isJuliaDigit(*next, base)) {
				i = next;

			} else {
				break;
			}

		} else {
			break;
		}
	}

	return i;
}


//
//	skipJuliaExponent
//

static TextEditor::Iterator skipJuliaExponent(TextEditor::Iterator i, TextEditor::Iterator end, bool hex) {
	// exponent is only consumed if digits follow (2e is 2 * e)
	if (i < end && (hex ? (*i == 'p' || *i == 'P') : (*i == 'e' || *i == 'E' || *i == 'f'))) {
		auto j = i;
		j++;

		if (j < end && (*j == '+' || *j == '-' || *j == 0x2212)) {
			j++;
		}

		if (j < end && isJuliaDigit(*j, 10)) {
			return skipJuliaDigits(j, end, 10);
		}
	}

	return i;
}


//
//	isJuliaDottableOperator
//

static bool isJuliaDottableOperator(ImWchar c) {
	return c == '!' || c == '%' || c == '&' || c == '*' || c == '+' || c == '-' || c == '/' || c == '<' ||
		c == '=' || c == '>' || c == '\\' || c == '^' || c == '|' || c == '~' || c == 0xf7 || c == 0x2212;
}


//
//	getJuliaNumber
//

static TextEditor::Iterator getJuliaNumber(TextEditor::Iterator start, TextEditor::Iterator end) {
	TextEditor::Iterator i = start;

	if (*i == '.') {
		// .5
		i++;

		if (i < end && isJuliaDigit(*i, 10)) {
			i = skipJuliaExponent(skipJuliaDigits(i, end, 10), end, false);

		} else {
			return start;
		}

	} else if (isJuliaDigit(*i, 10)) {
		auto next = i;
		next++;

		if (*i == '0' && next < end && (*next == 'x' || *next == 'b' || *next == 'o')) {
			// 0x1f, 0b101, 0o17 or hex float 0x1.8p3
			const int base = *next == 'x' ? 16 : (*next == 'b' ? 2 : 8);
			auto digits = next;
			digits++;

			if (digits < end && isJuliaDigit(*digits, base)) {
				i = skipJuliaDigits(digits, end, base);

				if (base == 16) {
					auto fraction = i;

					if (fraction < end && *fraction == '.') {
						fraction = skipJuliaDigits(++fraction, end, 16);
					}

					auto exponent = skipJuliaExponent(fraction, end, true);

					if (exponent != fraction) {
						i = exponent;
					}
				}

			} else {
				i = next;
			}

		} else {
			i = skipJuliaDigits(i, end, 10);

			// 1..2 and 1.+x don't have a fraction
			if (i < end && *i == '.') {
				next = i;
				next++;

				if (next >= end || (*next != '.' && !isJuliaDottableOperator(*next))) {
					i = skipJuliaDigits(next, end, 10);
				}
			}

			i = skipJuliaExponent(i, end, false);
		}

	} else {
		return start;
	}

	return i;
}


//
//	isJuliaStylePunctuation
//	!$%&'()*+,-./:;<=>?@[\]^{|}~ and Unicode operators
//

static bool isJuliaStylePunctuation(ImWchar character) {
	static const bool punctuation[128] = {
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false,  true, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
		false, false, false, false, false, false, false, false, false, false,  true,  true,  true,  true,  true,  true,
		 true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false,  true,  true,  true,  true, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false,  true,  true,  true,  true, false,
	};

	if (character < 127) {
		return punctuation[character];

	} else {
		return character == 0xac || character == 0xb1 || character == 0xd7 || character == 0xf7 || character == 0x2212 ||
			(character >= 0x2190 && character <= 0x2bff);
	}
}


//
//	matchesJuliaWord
//

static bool matchesJuliaWord(TextEditor::Iterator start, TextEditor::Iterator end, const char* word) {
	auto i = start;

	while (*word) {
		if (i >= end || *i != static_cast<ImWchar>(*word)) {
			return false;
		}

		i++;
		word++;
	}

	return i == end;
}


//
//	tokenizeJulia
//

static TextEditor::Iterator tokenizeJulia(TextEditor::Iterator start, TextEditor::Iterator end, TextEditor::Color& color) {
	auto i = start;
	const auto c = *i++;

	if (c == '@') {
		// macro call (@foo, @foo!, @.)
		if (i < end && *i == '.') {
			i++;

		} else {
			i = getJuliaIdentifier(i, end);
		}

		if (i - start > 1) {
			color = TextEditor::Color::preprocessor;
			return i;
		}

	} else if (c == ':') {
		if (i < end && *i == ':') {
			// type annotation
			color = TextEditor::Color::punctuation;
			return ++i;

		} else if (!followsJuliaOperand(start)) {
			// symbol (but not the range in 1:n)
			i = getJuliaIdentifier(i, end);

			if (i - start > 1) {
				color = TextEditor::Color::string;
				return i;
			}
		}

	} else if ((c == '<' || c == '>') && i < end && *i == ':') {
		// subtype operators (so T<:Real isn't seen as a symbol)
		color = TextEditor::Color::punctuation;
		return ++i;

	} else if (c == '\'') {
		if (followsJuliaOperand(start)) {
			// adjoint operator
			color = TextEditor::Color::punctuation;
			return i;

		} else if (i < end && *i == '\\') {
			// character literal with escape sequence ('\n', '\'')
			if (++i < end) {
				i++;
			}

			while (i < end && *i != '\'') {
				i++;
			}

			if (i < end) {
				color = TextEditor::Color::string;
				return ++i;
			}

		} else if (i < end && *i != '\'') {
			// character literal ('a')
			if (++i < end && *i == '\'') {
				color = TextEditor::Color::string;
				return ++i;
			}
		}

		color = TextEditor::Color::punctuation;
		return ++start;

	} else if (c == '`') {
		// single line command (``` commands are handled as strings)
		while (i < end && *i != '`') {
			if (*i++ == '\\' && i < end) {
				i++;
			}
		}

		color = TextEditor::Color::string;
		return i < end ? ++i : i;

	} else if (c == 'a' || c == 'p' || c == 'm') {
		// contextual keywords (abstract type, primitive type, mutable struct)
		auto wordEnd = getJuliaIdentifier(start, end);
		const char* second = nullptr;

		if (matchesJuliaWord(start, wordEnd, "abstract") || matchesJuliaWord(start, wordEnd, "primitive")) {
			second = "type";

		} else if (matchesJuliaWord(start, wordEnd, "mutable")) {
			second = "struct";
		}

		if (second) {
			i = wordEnd;

			while (i < end && (*i == ' ' || *i == '\t')) {
				i++;
			}

			if (i != wordEnd) {
				auto secondEnd = getJuliaIdentifier(i, end);

				if (matchesJuliaWord(i, secondEnd, second)) {
					color = TextEditor::Color::declaration;
					return secondEnd;
				}
			}
		}
	}

	return start;
}


//
//	GetLanguageDefinitionJulia
//

static const TextEditor::Language* GetLanguageDefinitionJulia() {
	static bool initialized = false;
	static TextEditor::Language language;

	if (!initialized) {
		language.name = "Julia";
		language.singleLineComment = "#";
		language.commentStart = "#=";
		language.commentEnd = "=#";
		language.nestedComments = true;
		language.hasDoubleQuotedStrings = true;
		language.otherStringStart = "\"\"\"";
		language.otherStringEnd = "\"\"\"";
		language.otherStringAltStart = "```";
		language.otherStringAltEnd = "```";
		language.stringEscape = '\\';

		static const char* const keywords[] = {
			"begin", "break", "catch", "continue", "do", "else", "elseif", "end", "export", "false", "finally", "for",
			"if", "import", "in", "isa", "let", "quote", "return", "true", "try", "using", "where", "while"
		};

		static const char* const declarations[] = {
			"baremodule", "const", "function", "global", "local", "macro", "module", "struct",
			"AbstractArray", "AbstractChar", "AbstractDict", "AbstractFloat", "AbstractMatrix", "AbstractRange",
			"AbstractSet", "AbstractString", "AbstractVector", "Any", "Array", "BigFloat", "BigInt", "Bool", "Char",
			"Complex", "ComplexF32", "ComplexF64", "DataType", "Dict", "Exception", "Expr", "Float16", "Float32",
			"Float64", "Function", "Int", "Int128", "Int16", "Int32", "Int64", "Int8", "Integer", "IO", "Matrix",
			"Missing", "Module", "NamedTuple", "Nothing", "Number", "Pair", "Ptr", "Rational", "Real", "Ref", "Set",
			"Signed", "String", "Symbol", "Tuple", "Type", "UInt", "UInt128", "UInt16", "UInt32", "UInt64", "UInt8",
			"Union", "UnionAll", "Unsigned", "Vector", "Val"
		};

		static const char* const identifiers[] = {
			"ARGS", "C_NULL", "ENV", "Inf", "Inf16", "Inf32", "Inf64", "NaN", "NaN16", "NaN32", "NaN64", "VERSION",
			"im", "missing", "nothing", "pi", "stderr", "stdin", "stdout", "π", "ℯ",
			"abs", "all", "any", "append!", "collect", "convert", "copy", "deepcopy", "delete!", "eachindex", "eltype",
			"empty!", "enumerate", "error", "filter", "findall", "findfirst", "first", "get", "getfield", "getindex",
			"haskey", "isempty", "isnothing", "keys", "last", "length", "map", "max", "maximum", "min", "minimum",
			"ndims", "pairs", "pop!", "print", "println", "promote", "push!", "reduce", "repr", "resize!", "reverse",
			"setfield!", "setindex!", "show", "similar", "size", "sort", "sort!", "string", "sum", "throw", "typeof",
			"values", "zeros", "ones", "zip"
		};

		for (auto& keyword : keywords) { language.keywords.insert(keyword); }
		for (auto& declaration : declarations) { language.declarations.insert(declaration); }
		for (auto& identifier : identifiers) { language.identifiers.insert(identifier); }

		language.isPunctuation = isJuliaStylePunctuation;
		language.getIdentifier = getJuliaIdentifier;
		language.getNumber = getJuliaNumber;
		language.customTokenizer = tokenizeJulia;
		initialized = true;
	}

	return &language;
}
