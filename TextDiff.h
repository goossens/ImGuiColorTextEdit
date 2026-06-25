//	TextDiff - A syntax highlighting text diff widget for Dear ImGui.
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "TextEditor.h"


//
//	TextDiff
//

class IMGUI_API TextDiff {
public:
	// constructor
	TextDiff();

	// specify options
	inline void SetSideBySideMode(bool flag) { diff.sideBySideMode = flag; }
	inline bool GetSideBySideMode() const { return diff.sideBySideMode; }
	inline void SetTabSize(size_t value) { diff.config.tabSize = value; }
	inline size_t GetTabSize() const { return diff.config.tabSize; }
	inline void SetLineSpacing(float value) { diff.config.lineSpacing = std::max(1.0f, std::min(2.0f, value)); }
	inline float GetLineSpacing() const { return diff.config.lineSpacing; }
	inline void SetWordWrapEnabled(bool value) { diff.config.wordWrap = value; }
	inline bool IsWordWrapEnabled() const { return diff.config.wordWrap; }
	inline void SetShowWhitespacesEnabled(bool value) { diff.config.showSpaces = value; diff.config.showTabs = value; }
	inline bool IsShowWhitespacesEnabled() const { return diff.config.showSpaces && diff.config.showTabs; }
	inline void SetShowSpacesEnabled(bool value) { diff.config.showSpaces = value; }
	inline bool IsShowSpacesEnabled() const { return diff.config.showSpaces; }
	inline void SetShowTabsEnabled(bool value) { diff.config.showTabs = value; }
	inline bool IsShowTabsEnabled() const { return diff.config.showTabs; }
	inline void SetShowScrollbarMiniMapEnabled(bool value) { diff.config.showScrollbarMiniMap = value; }
	inline bool IsShowScrollbarMiniMapEnabled() const { return diff.config.showScrollbarMiniMap; }
	inline void SetLanguage(const TextEditor::Language* language) { diff.config.language = language; }
	inline const TextEditor::Language* GetLanguage() const { return diff.config.language; };
	inline void SetColors(ImU32 ac, ImU32 dc) { diff.addedColor = ac; diff.deletedColor = dc; }

	inline void SetPalette(const TextEditor::Palette& newPalette) { diff.paletteBase = newPalette; diff.paletteAlpha = -1.0f; }
	inline const TextEditor::Palette& GetPalette() const { return diff.paletteBase; }

	// programmatically set focus on the editor
	inline void SetFocus() { diff.focusOnDiff = true; }

	// specify the text to be compared (using UTF-8 encoded strings)
	void SetText(const std::string_view& left, const std::string_view& right);

	// render text diff in a Dear ImGui context
	void Render(const char* title, const ImVec2& size=ImVec2(), bool border=false);

private:
	// information about a single line
	enum class DiffType {
		common,
		added,
		deleted
	};

	class LineState {
	public:
		LineState(size_t leftLine, size_t rightLine, DiffType type) : leftLine(leftLine), rightLine(rightLine), type(type) {}
		size_t leftLine;
		size_t rightLine;
		DiffType type;
	};

	// diff context
	struct Diff {
		// options
		bool sideBySideMode = false;
		bool previousSideBySideMode = false;
		TextEditor::Config config;
		bool focusOnDiff = false;

		// the two documents being compared
		TextEditor::Document leftDocument;
		TextEditor::Document rightDocument;

		// coloring supports
		TextEditor::Colorizer leftColorizer;
		TextEditor::Colorizer rightColorizer;

		// difference between the two documents
		std::vector<LineState> state;

		// colors
		ImU32 addedColor = IM_COL32(0, 150, 32, 128);
		ImU32 deletedColor = IM_COL32(180, 0, 32, 128);

		// color palette support
		TextEditor::Palette paletteBase;
		TextEditor::Palette palette;
		float paletteAlpha;
	} diff;

	// integrated rendering state
	struct IntegratedView {
		// visual layout
		struct Row {
			Row() = default;

			Row(DiffType type, size_t leftLine, size_t rightLine, size_t section, size_t columns) :
				type(type), leftLine(leftLine), rightLine(rightLine), section(section), columns(columns) {}

			DiffType type;
			size_t leftLine;
			size_t rightLine;
			size_t section;
			size_t columns;
		};

		std::vector<Row> rows;

		// rendering context
		ImVec2 cursorScreenPos;

		ImFont* font;
		float fontSize;
		ImVec2 glyphSize;

		int leftLineNumberDigits;
		int rightLineNumberDigits;
		float lineNumberWidth;
		float textColumnWidth;

		float lineNumberPos;
		float textPos;
		float textEnd;
		float textScroll;

		size_t visibleRows;
		size_t visibleColumns;
		size_t maxColumns = 0;

		size_t firstVisibleRow;
		size_t lastVisibleRow;
		size_t firstVisibleColumn;
		size_t lastVisibleColumn;
		size_t firstRenderableColumn;

		// layout support
		TextEditor::LineFold leftLineFold;
		TextEditor::LineFold rightLineFold;
		TextEditor::TypeSetter leftTypeSetter;
		TextEditor::TypeSetter rightTypeSetter;

		// rendering functions
		void render(const char* title, const ImVec2& size, bool border, Diff& diff);
		void renderBackground(const Diff& diff);
		void renderText(const Diff& diff);
		void renderLine(float x, float y, const TextEditor::Line& line, size_t section, const Diff& diff);
		void renderScrollbar();
		void renderMiniMap(const Diff& diff);

		// layout functions
		void updateLayout(Diff& diff);
	} integratedView;

	// side-by-side rendering state
	struct SideBySideView {
		// visual layout
		struct Row {
			Row() = default;

			Row(DiffType type, size_t leftLine, size_t leftSection, size_t rightLine, size_t rightSection, size_t columns) :
				type(type),
				leftLine(leftLine), leftSection(leftSection),
				rightLine(rightLine), rightSection(rightSection),
				columns(columns) {}

			DiffType type;
			size_t leftLine;
			size_t leftSection;
			size_t rightLine;
			size_t rightSection;
			size_t columns;
		};

		std::vector<Row> rows;

		// rendering context
		ImVec2 cursorScreenPos;

		ImFont* font;
		float fontSize;
		ImVec2 glyphSize;

		int leftLineNumberDigits;
		int rightLineNumberDigits;
		float leftLineNumberWidth;
		float rightLineNumberWidth;
		float textColumnWidth;

		float leftLineNumberPos;
		float leftTextPos;
		float rightLineNumberPos;
		float rightTextPos;
		float rightTextEnd;
		float textScroll;

		size_t visibleRows;
		size_t visibleColumns;
		size_t maxColumns = 0;

		size_t firstVisibleRow;
		size_t lastVisibleRow;
		size_t firstVisibleColumn;
		size_t lastVisibleColumn;
		size_t firstRenderableColumn;

		// layout support
		TextEditor::LineFold leftLineFold;
		TextEditor::LineFold rightLineFold;
		TextEditor::TypeSetter leftTypeSetter;
		TextEditor::TypeSetter rightTypeSetter;

		// rendering functions
		void render(const char* title, const ImVec2& size, bool border, Diff& diff);
		void renderBackground(const Diff& diff);
		void renderText(const Diff& diff);
		void renderLine(float x, float y, const TextEditor::Line& line, size_t section, const Diff& diff);
		void renderScrollbars();
		void renderMiniMap(const Diff& diff);

		// layout functions
		void updateLayout(Diff& diff);
	} sideBySideView;

	// split string into lines
	static void splitLines(std::vector<std::string_view>& result, const std::string_view& text);

	// update the color palette
	void updatePalette();
};
