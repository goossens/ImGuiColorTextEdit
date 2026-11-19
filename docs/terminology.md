# Terminology

To provide more clarity in terms of the terminology used in this text editor, a set
of definitions is required to create a common understanding and to ensure they can be
implemented consistently across the editor's components. The terminology described here
is not always consistent with other editors or IDEs but if the rest of the world agreed
on a single set of definitions, we wouldn't need to do this. The definitions
presented here must therefore be seen as a compromise.

## Glyphs and Codepoints

A glyph is the specific shape, design, or representation of a character. In this editor,
a glyph is represented by a Unicode codepoint that can be either 16 or 32 bits depending
on how Dear ImGui is configured. If IMGUI_USE_WCHAR32 is defined, the editor uses 32
bit codepoints which can express all glyph in the [Unicode Character Database](https://www.unicode.org/ucd/).
If IMGUI_USE_WCHAR32 is not defined (which is the default), 16 bits are used for each
codepoint which basically limits unicode support to the Basic Multilingual Plane (see
[this article on Wikipedia](https://en.wikipedia.org/wiki/Plane_(Unicode))).

## Glyph Sequences by Category

For some operations in the editor (e.g. move cursor left/right or double click to select text),
accelerators are available that skip multiple glyphs based on the following predefined glyph categories:

- **Whitespace** - Sequence of unicode codepoints that are marked as White_Space
in the PropList.txt table of the Unicode Character Database.
- **Word** - Sequence of unicode codepoints that are marked as letters, numbers or the underscore
in the UnicodeData.txt table of the Unicode Character Database. A language definition can override
the definition of a word and include other codepoints as required.
- **Others** - Sequence of unicode codepoints that are not marked as whitespace or word glyphs.

## Language Specific Glyph Sequences

The following sequences of glyphs are detected by language specifications and are used
for coloring and/or information discovery (e.g. get identifier at cursor).

- **Keyword** - Sequence of unicode codepoints marked as such by the current language definition. These are typically used for reserved words in a language.
- **Declaration** - Sequence of unicode codepoints marked as such by the current language definition. These are typically used by typed languages for builtin types or keywords to declare a type.
- **Comment** - Sequence of unicode codepoints marked as such by the current language definition. These can be single line or multi-line depending on the language. Some languages even support nested comments.
- **String** - Sequence of unicode codepoints marked as such by the current language definition. These can be single line or multi-line depending on the language. Typically languages use codeword sequences to mark the start and end of a string and some languages support multi-line strings or even nested strings. The start and end sequence are considered part of the string.
- **Number** - Sequence of unicode codepoints marked as such by the current language definition. These can be integers, floats, have non-decimal notations or have scientific notations depending on the language.
- **Identifier** - Sequence of unicode codepoints marked as such by the current language definition.
- **Qualified Name** - A sequence of identifiers separated by language specific membership operators.
- **Punctuation** - Unicode codepoints that are used as language specific operators.

## Examples

- **Geography** and **test_system5** are words whether they appear in comments, in a string or in code.
- **Geography** and **test_system5** are only identifiers when they appear in code.
- **3.14f** is recognized as a word and a number (if not in comments or a string) but not as an identifier.
- **std::filesystems::path** are three words separated by other glyphs. It is also a sequence of three identifiers with two qualified separators in C++ (if not in comments or a string). The entire sequence is recognized as a qualified identifier.

## API calls to help you with Information Discovery

A number of API calls are available to discover what is preset at a certain document position (DocPos).
First there is a set of call to determine if the glyph at a DocPos belongs to a certain sequence.
Please not that DocPos coordinates are zero-based so visually you get the information about the
glyph right of the position.

```c++
	bool IsWord(DocPos pos) const;
	bool IsKeyword(DocPos pos) const;
	bool IsDeclaration(DocPos pos) const;
	bool IsIdentifier(DocPos pos) const;
	bool IsNumber(DocPos pos) const;
	bool IsString(DocPos pos) const;
	bool IsWhiteSpace(DocPos pos) const;
	bool IsPunctuation(DocPos pos) const;
	bool IsComment(DocPos pos) const;
```

API calls are also available to extract a required sequence of glyphs as a string. If the glyph at the provided
DocPos does not have the right characteristics, an empty string is returned.

```c++
	std::string GetWord(DocPos pos) const;
	std::string GetKeyword(DocPos pos) const;
	std::string GetDeclaration(DocPos pos) const;
	std::string GetIdentifier(DocPos pos) const;
	std::string GetNumber(DocPos pos) const;
	std::string GetString(DocPos pos) const;
	std::string GetWhiterSpace(DocPos pos) const;
	std::string GetPunctuation(DocPos pos) const;
	std::string GetComment(DocPos pos) const;
	std::string GetQualifiedIdentifier(DocPos pos) const;
```

API calls are also available to inspect single glyphs or all glyphs on a line.
This should enable almost any custom algorithm in the application.

```c++
	std::vector<ImWchar> GetLineCodePoints(size_t line) const;
	ImWchar GetCodePoint(DocPos location) const;

	std::vector<Color> GetLineColors(size_t line) const;
	Color GetColor(DocPos location) const;
```
