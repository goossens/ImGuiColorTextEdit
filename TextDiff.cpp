//	TextDiff - A syntax highlighting text diff widget for Dear ImGui.
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <cmath>

#include "imgui_internal.h"

#include "dtl.h"
#include "TextDiff.h"


//
//	TextDiff::TextDiff
//

TextDiff::TextDiff() {
	SetPalette(TextEditor::GetDefaultPalette());
}


//
//	TextDiff::SetText
//

void TextDiff::SetText(const std::string_view& left, const std::string_view& right) {
	// split text into lines
	std::vector<std::string_view> leftLines;
	std::vector<std::string_view> rightLines;
	splitLines(leftLines, left);
	splitLines(rightLines, right);

	// create two documents
	diff.leftDocument.setText(diff.config, leftLines);
	diff.rightDocument.setText(diff.config, rightLines);

	// calculate the difference between the two documents
	dtl::Diff<std::string_view> difference(leftLines, rightLines);
	difference.compose();
	auto ranges = difference.getSes().getSequence();

	// determine status of each line in diff view
	diff.state.clear();
	size_t leftIndex = 0;
	size_t rightIndex = 0;

	for (auto& [text, info] : ranges) {
		switch (info.type) {
			case dtl::SES_COMMON:
				diff.state.emplace_back(leftIndex++, rightIndex++, DiffType::common);
				break;

			case dtl::SES_ADD:
				diff.state.emplace_back(leftIndex, rightIndex++, DiffType::added);
				break;

			case dtl::SES_DELETE:
				diff.state.emplace_back(leftIndex++, rightIndex, DiffType::deleted);
				break;
		}
	}
}


//
//	TextDiff::Render
//

void TextDiff::Render(const char* title, const ImVec2& size, bool border) {
	// update color palette (if required)
	if (diff.paletteAlpha != ImGui::GetStyle().Alpha) {
		updatePalette();
	}

	// update colorizer states (if required)
	diff.leftColorizer.update(diff.config, diff.leftDocument);
	diff.rightColorizer.update(diff.config, diff.rightDocument);

	// ensure editor has focus (if required)
	if (diff.focusOnDiff) {
		ImGui::SetNextWindowFocus();
		diff.focusOnDiff = false;
	}

	if (diff.sideBySideMode) {
		// render a custom side-by-side view
		sideBySideView.render(title, size, border, diff);

	} else {
		// render combined view
		integratedView.render(title, size, border, diff);
	}

	// reset view mode
	diff.previousSideBySideMode = diff.sideBySideMode;

	// reset document state (if required)
	if (diff.leftDocument.isUpdated()) {
		diff.leftDocument.resetUpdated();
	}

	if (diff.rightDocument.isUpdated()) {
		diff.rightDocument.resetUpdated();
	}
}


//
//	TextDiff::IntegratedView::render
//

void TextDiff::IntegratedView::render(const char* title, const ImVec2& size, bool border, Diff& diff) {
	// get font information
	font = ImGui::GetFont();
	fontSize = ImGui::GetFontSize();
	glyphSize = ImVec2(ImGui::CalcTextSize("#").x, ImGui::GetTextLineHeightWithSpacing() * diff.config.lineSpacing);

	// determine flags
	ImGuiChildFlags childFlags = border ? ImGuiChildFlags_Borders : ImGuiChildFlags_None;

	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_HorizontalScrollbar |
		ImGuiWindowFlags_NoNavInputs |
		ImGuiWindowFlags_NoMove;

		// start rendering the widget
	ImGui::SetNextWindowContentSize(ImVec2(0.0f, glyphSize.y * rows.size()));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(diff.palette.get(TextEditor::Color::background)));
	ImGui::BeginChild(title, size, childFlags, windowFlags);

	// determine visible dimensions
	cursorScreenPos = ImGui::GetCursorScreenPos();
	auto visibleSize = ImGui::GetContentRegionAvail();

	leftLineNumberDigits = static_cast<int>(std::log10(diff.leftDocument.size() + 1) + 1.0f);
	rightLineNumberDigits = static_cast<int>(std::log10(diff.rightDocument.size() + 1) + 1.0f);
	lineNumberWidth = glyphSize.x * (leftLineNumberDigits + rightLineNumberDigits + 3);

	textColumnWidth = visibleSize.x - lineNumberWidth;
	textColumnWidth = std::floor(textColumnWidth / glyphSize.x) * glyphSize.x;
	diff.config.wordWrapColumns = static_cast<size_t>(std::max(textColumnWidth / glyphSize.x, 0.0f));

	// update typesetters and update layout (if required)
	bool layoutChanged = false;
	layoutChanged |= leftTypeSetter.update(diff.config, diff.leftDocument, leftLineFold);
	layoutChanged |= rightTypeSetter.update(diff.config, diff.rightDocument, rightLineFold);
	bool viewChanged = diff.sideBySideMode != diff.previousSideBySideMode;

	if (layoutChanged || viewChanged) {
		updateLayout(diff);
	}

	// determine layout parameters
	lineNumberPos = cursorScreenPos.x;
	textPos = lineNumberPos + lineNumberWidth;
	textEnd = textPos + textColumnWidth;

	visibleRows = std::max(static_cast<int>(std::ceil(visibleSize.y / glyphSize.y)), 0);
	visibleColumns = std::max(static_cast<int>(std::ceil(textColumnWidth / glyphSize.x)), 0);

	firstVisibleRow = std::max(static_cast<int>(std::floor(ImGui::GetScrollY() / glyphSize.y)), 0);
	lastVisibleRow = std::min(static_cast<int>(std::ceil((ImGui::GetScrollY() + visibleSize.y) / glyphSize.y)), static_cast<int>(rows.size() - 1));
	firstVisibleColumn = std::max(static_cast<int>(std::floor(textScroll / glyphSize.x)), 0);
	lastVisibleColumn = static_cast<int>(std::ceil((textScroll + textColumnWidth) / glyphSize.x));
	firstRenderableColumn = (firstVisibleColumn / diff.config.tabSize) * diff.config.tabSize;

	renderBackground(diff);
	renderText(diff);
	renderScrollbar();
	renderMiniMap(diff);

	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}


//
//	TextDiff::IntegratedView::renderBackground
//

void TextDiff::IntegratedView::renderBackground(const Diff& diff) {
	// render line numbers and text backgrounds
	auto drawList = ImGui::GetWindowDrawList();
	auto y = cursorScreenPos.y + firstVisibleRow * glyphSize.y;
	char buffer[32];

	for (auto i = firstVisibleRow; i <= lastVisibleRow; i++) {
		const auto& row = rows[i];

		auto lineLeft = static_cast<int>(row.leftLine + 1);
		auto lineRight = static_cast<int>(row.rightLine + 1);

		switch(row.type) {
			case DiffType::common:
				if (row.section == 0) {
					snprintf(buffer, sizeof(buffer), " %*d %*d", leftLineNumberDigits, lineLeft, rightLineNumberDigits, lineRight);
					drawList->AddText(ImVec2(lineNumberPos, y), diff.palette.get(TextEditor::Color::lineNumber), buffer);
				}

				break;

			case DiffType::added:
				if (row.section == 0) {
					snprintf(buffer, sizeof(buffer), " %*s %*d +", leftLineNumberDigits, "", rightLineNumberDigits, lineRight);
					drawList->AddText(ImVec2(lineNumberPos, y), diff.palette.get(TextEditor::Color::lineNumber), buffer);
				}

				drawList->AddRectFilled(ImVec2(lineNumberPos, y),ImVec2(textEnd, y + glyphSize.y), diff.addedColor);
				break;

			case DiffType::deleted:
				if (row.section == 0) {
					snprintf(buffer, sizeof(buffer), " %*d %*s -", leftLineNumberDigits, lineLeft, rightLineNumberDigits, "");
					drawList->AddText(ImVec2(lineNumberPos, y), diff.palette.get(TextEditor::Color::lineNumber), buffer);
				}

				drawList->AddRectFilled(ImVec2(lineNumberPos, y), ImVec2(textEnd, y + glyphSize.y), diff.deletedColor);

				break;
		}

		y += glyphSize.y;
	}
}


//
//	TextDiff::IntegratedView::renderText
//

void TextDiff::IntegratedView::renderText(const Diff& diff) {
	// setup rendering
	auto drawList = ImGui::GetWindowDrawList();
	auto yTop = drawList->GetClipRectMin().y;
	auto yBottom = drawList->GetClipRectMax().y;

	if (maxColumns * glyphSize.x > textColumnWidth) {
		yBottom -= ImGui::GetStyle().ScrollbarSize + ImGui::GetCurrentWindow()->WindowBorderSize;
	}

	// render left text
	drawList->PushClipRect(ImVec2(textPos, yTop), ImVec2(textEnd, yBottom), false);

	for (auto i = firstVisibleRow; i <= lastVisibleRow; i++) {
		const auto& row = rows[i];
		auto y = cursorScreenPos.y + i * glyphSize.y;

		switch(row.type) {
			case DiffType::common:
			case DiffType::deleted:
				renderLine(textPos, y, diff.leftDocument[row.leftLine], row.section, diff);
				break;

			case DiffType::added:
				renderLine(textPos, y, diff.rightDocument[row.rightLine], row.section, diff);
				break;
		}
	}

	drawList->PopClipRect();
}


//
//	TextDiff::IntegratedView::renderLine
//

void TextDiff::IntegratedView::renderLine(float x, float y, const TextEditor::Line& line, size_t sectionNo, const Diff& diff) {
	// draw colored glyphs for specified line
	auto drawList = ImGui::GetWindowDrawList();

	// determine visible boundaries for this row
	size_t index;
	size_t column;
	size_t endColumn;

	if (diff.config.wordWrap && line.sections) {
		const auto& section = line.sections->at(sectionNo);
		index = section.startIndex;
		column = section.indent;
		endColumn = section.columns;

	} else {
		index = 0;
		column = 0;
		endColumn = line.columns;
	}

	// only process all visible columns
	while (column < endColumn && column <= lastVisibleColumn) {
		auto& glyph = line[index++];
		auto codepoint = glyph.codepoint;
		ImVec2 glyphPos(x + column * glyphSize.x - textScroll, y);

		// handle tabs
		if (codepoint == '\t') {
			if (diff.config.showTabs && column >= firstRenderableColumn) {
				const auto x1 = glyphPos.x + glyphSize.x * 0.3f;
				const auto y1 = glyphPos.y + fontSize * 0.5f;
				const auto x2 = glyphPos.x + glyphSize.x;

				ImVec2 p1, p2, p3, p4;
				p1 = ImVec2(x1, y1);
				p2 = ImVec2(x2, y1);
				p3 = ImVec2(x2 - fontSize * 0.16f, y1 - fontSize * 0.16f);
				p4 = ImVec2(x2 - fontSize * 0.16f, y1 + fontSize * 0.16f);

				drawList->AddLine(p1, p2, diff.palette.get(TextEditor::Color::whitespace));
				drawList->AddLine(p2, p3, diff.palette.get(TextEditor::Color::whitespace));
				drawList->AddLine(p2, p4, diff.palette.get(TextEditor::Color::whitespace));
			}

			column += diff.config.tabSize - (column % diff.config.tabSize);

		// handle spaces
		} else if (codepoint == ' ') {
			if (diff.config.showSpaces && column >= firstRenderableColumn) {
				const auto x1 = glyphPos.x + glyphSize.x * 0.5f;
				const auto y1 = glyphPos.y + fontSize * 0.5f;
				drawList->AddCircleFilled(ImVec2(x1, y1), 1.5f, diff.palette.get(TextEditor::Color::whitespace), 4);
			}

			column++;

		// handle regular glyphs
		} else {
			if (column >= firstRenderableColumn) {
				font->RenderChar(drawList, fontSize, glyphPos, diff.palette.get(glyph.color), codepoint);
			}

			column++;
		}
	}
}


//
//	TextDiff::IntegratedView::renderScrollbar
//

void TextDiff::IntegratedView::renderScrollbar() {
	auto maxColumnsWidth = maxColumns * glyphSize.x;

	if (maxColumnsWidth > textColumnWidth) {
		const ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImRect outerRect = window->Rect();
		auto borderSize = std::round(window->WindowBorderSize * 0.5f);
		auto scrollbarSize = ImGui::GetStyle().ScrollbarSize;

		auto scrollbarTop = std::max(outerRect.Min.y + borderSize, outerRect.Max.y - borderSize - scrollbarSize);
		ImRect scrollbarFrame(textPos, scrollbarTop, textEnd, scrollbarTop + scrollbarSize);
		ImS64 scroll = static_cast<ImS64>(textScroll);

		if (ImGui::ScrollbarEx(
			scrollbarFrame,
			ImGui::GetID("textScroll"),
			ImGuiAxis_X,
			&scroll,
			static_cast<ImS64>(textColumnWidth),
			static_cast<ImS64>(maxColumnsWidth),
			ImDrawFlags_RoundCornersAll)) {

			textScroll = static_cast<float>(scroll);
		}

		if (ImGui::IsWindowFocused()) {
			textScroll = std::clamp(textScroll - ImGui::GetIO().MouseWheelH * ImGui::GetFontSize(), 0.0f, maxColumnsWidth - textColumnWidth);

			if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
				textScroll = std::max(textScroll - glyphSize.x, 0.0f);

			} else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
				textScroll = std::min(textScroll + glyphSize.x, maxColumnsWidth - textColumnWidth);

			} else if (ImGui::IsKeyPressed(ImGuiKey_Home)) {
				textScroll = 0.0f;

			} else if (ImGui::IsKeyPressed(ImGuiKey_End)) {
				textScroll = maxColumnsWidth - textColumnWidth;
			}
		}

	} else {
		// no scrolling if the text fits
		textScroll = 0.0f;
	}

	if (ImGui::IsWindowFocused()) {
		if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
			if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
				ImGui::SetScrollY(0.0f);

			} else {
				ImGui::SetScrollY(std::max(ImGui::GetScrollY() - glyphSize.y, 0.0f));
			}

		} else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
			if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
				ImGui::SetScrollY(ImGui::GetScrollMaxY());

			} else {
				ImGui::SetScrollY(std::min(ImGui::GetScrollY() + glyphSize.y, ImGui::GetScrollMaxY()));
			}

		} else if (ImGui::IsKeyPressed(ImGuiKey_PageUp)) {
			ImGui::SetScrollY(std::max(ImGui::GetScrollY() - (visibleRows - 2) * glyphSize.y, 0.0f));

		} else if (ImGui::IsKeyPressed(ImGuiKey_PageDown)) {
			ImGui::SetScrollY(std::min(ImGui::GetScrollY() + (visibleRows - 2) * glyphSize.y, ImGui::GetScrollMaxY()));
		}
	}
}


//
//	TextDiff::IntegratedView::renderMiniMap
//

void TextDiff::IntegratedView::renderMiniMap(const Diff& diff) {
	// based on https://github.com/ocornut/imgui/issues/3114
	if (diff.config.showScrollbarMiniMap) {
		auto window = ImGui::GetCurrentWindow();

		if (window->ScrollbarY) {
			auto drawList = ImGui::GetWindowDrawList();
			auto rect = ImGui::GetWindowScrollbarRect(window, ImGuiAxis_Y);
			auto rowHeight = rect.GetHeight() / static_cast<float>(rows.size());
			auto offset = (rect.Max.x - rect.Min.x) * 0.3f;
			auto left = rect.Min.x + offset;
			auto right = rect.Max.x - offset;

			drawList->PushClipRect(rect.Min, rect.Max, false);

			// render diff locations
			for (size_t i = 0; i < rows.size(); i++) {
				const auto& row = rows[i];

				if (row.type != DiffType::common) {
					auto color = (row.type == DiffType::added) ? diff.addedColor : diff.deletedColor;
					auto ly = std::round(rect.Min.y + i * rowHeight);
					drawList->AddRectFilled(ImVec2(left, ly), ImVec2(right, ly + rowHeight), color);
				}
			}

			drawList->PopClipRect();
		}
	}
}


//
//	TextDiff::IntegratedView::updateLayout
//

void TextDiff::IntegratedView::updateLayout(Diff& diff) {
	rows.clear();
	maxColumns = 0;

	for (auto& lineState : diff.state) {
		auto& line = lineState.type == DiffType::added
			? diff.rightDocument[lineState.rightLine]
			: diff.leftDocument[lineState.leftLine];

		if (line.rows > 10) {
			line.rows = 10;
		}

		for (size_t i = 0; i < line.rows; i++) {
			rows.emplace_back(lineState.type, lineState.leftLine, lineState.rightLine, i, line.columns);
		}

		maxColumns = std::max(maxColumns, line.columns);
	}
}


//
//	TextDiff::SideBySideView::render
//

void TextDiff::SideBySideView::render(const char* title, const ImVec2& size, bool border, Diff& diff) {
	// get font information
	font = ImGui::GetFont();
	fontSize = ImGui::GetFontSize();
	glyphSize = ImVec2(ImGui::CalcTextSize("#").x, ImGui::GetTextLineHeightWithSpacing() * diff.config.lineSpacing);

	// determine flags
	ImGuiChildFlags childFlags = border ? ImGuiChildFlags_Borders : ImGuiChildFlags_None;

	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_HorizontalScrollbar |
		ImGuiWindowFlags_NoNavInputs;

		// start rendering the widget
	ImGui::SetNextWindowContentSize(ImVec2(0.0f, glyphSize.y * rows.size()));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(diff.palette.get(TextEditor::Color::background)));
	ImGui::BeginChild(title, size, childFlags, windowFlags);

	// determine visible dimensions
	cursorScreenPos = ImGui::GetCursorScreenPos();
	auto visibleSize = ImGui::GetContentRegionAvail();

	leftLineNumberDigits = static_cast<int>(std::log10(diff.leftDocument.size() + 1) + 1.0f);
	rightLineNumberDigits = static_cast<int>(std::log10(diff.rightDocument.size() + 1) + 1.0f);
	leftLineNumberWidth = glyphSize.x * (leftLineNumberDigits + 4);
	rightLineNumberWidth = glyphSize.x * (rightLineNumberDigits + 4);

	textColumnWidth = static_cast<float>(visibleSize.x - leftLineNumberWidth - rightLineNumberWidth) / 2.0f;
	textColumnWidth = std::floor(textColumnWidth / glyphSize.x) * glyphSize.x;
	diff.config.wordWrapColumns = static_cast<size_t>(std::max(textColumnWidth / glyphSize.x, 0.0f));

	// update typesetters and update layout (if required)
	bool layoutChanged = false;
	layoutChanged |= leftTypeSetter.update(diff.config, diff.leftDocument, leftLineFold);
	layoutChanged |= rightTypeSetter.update(diff.config, diff.rightDocument, rightLineFold);
	bool viewChanged = diff.sideBySideMode != diff.previousSideBySideMode;

	if (layoutChanged || viewChanged) {
		updateLayout(diff);
	}

	// determine layout parameters
	leftLineNumberPos = cursorScreenPos.x;
	leftTextPos = leftLineNumberPos + leftLineNumberWidth;
	rightLineNumberPos = leftTextPos + textColumnWidth;
	rightTextPos = rightLineNumberPos + rightLineNumberWidth;
	rightTextEnd = rightTextPos + textColumnWidth;

	visibleRows = std::max(static_cast<int>(std::ceil(visibleSize.y / glyphSize.y)), 0);
	visibleColumns = std::max(static_cast<int>(std::ceil(textColumnWidth / glyphSize.x)), 0);

	firstVisibleRow = std::max(static_cast<int>(std::floor(ImGui::GetScrollY() / glyphSize.y)), 0);
	lastVisibleRow = std::min(static_cast<int>(std::floor((ImGui::GetScrollY() + visibleSize.y) / glyphSize.y)), static_cast<int>(rows.size() - 1));
	firstVisibleColumn = std::max(static_cast<int>(std::floor(textScroll / glyphSize.x)), 0);
	lastVisibleColumn = static_cast<int>(std::floor((textScroll + textColumnWidth) / glyphSize.x));
	firstRenderableColumn = (firstVisibleColumn / diff.config.tabSize) * diff.config.tabSize;

	renderBackground(diff);
	renderText(diff);
	renderScrollbars();
	renderMiniMap(diff);

	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}


//
//	TextDiff::SideBySideView::renderBackground
//

void TextDiff::SideBySideView::renderBackground(const Diff& diff) {
	// render line numbers and text backgrounds
	auto drawList = ImGui::GetWindowDrawList();
	auto y = cursorScreenPos.y + firstVisibleRow * glyphSize.y;
	char buffer[32];

	for (auto i = firstVisibleRow; i <= lastVisibleRow; i++) {
		const auto& row = rows[i];

		auto lineLeft = static_cast<int>(row.leftLine + 1);
		auto lineRight = static_cast<int>(row.rightLine + 1);

		switch(row.type) {
			case DiffType::common:
				if (row.leftSection == 0) {
					snprintf(buffer, sizeof(buffer), " %*d", leftLineNumberDigits, lineLeft);
					drawList->AddText(ImVec2(leftLineNumberPos, y), diff.palette.get(TextEditor::Color::lineNumber), buffer);
					snprintf(buffer, sizeof(buffer), " %*d", rightLineNumberDigits, lineRight);
					drawList->AddText(ImVec2(rightLineNumberPos, y), diff.palette.get(TextEditor::Color::lineNumber), buffer);
				}

				break;

			case DiffType::added:
				if (row.rightSection == 0) {
					snprintf(buffer, sizeof(buffer), " %*d +", rightLineNumberDigits, lineRight);
					drawList->AddText(ImVec2(rightLineNumberPos, y), diff.palette.get(TextEditor::Color::lineNumber), buffer);
				}

				drawList->AddRectFilled(
					ImVec2(rightTextPos, y),
					ImVec2(rightTextEnd, y + glyphSize.y),
					diff.addedColor);

				break;

			case DiffType::deleted:
				if (row.leftSection == 0) {
					snprintf(buffer, sizeof(buffer), " %*d -", leftLineNumberDigits, lineLeft);
					drawList->AddText(ImVec2(leftLineNumberPos, y), diff.palette.get(TextEditor::Color::lineNumber), buffer);
				}

				drawList->AddRectFilled(
					ImVec2(leftTextPos, y),
					ImVec2(rightLineNumberPos, y + glyphSize.y),
					diff.deletedColor);

				break;
		}

		y += glyphSize.y;
	}
}


//
//	TextDiff::SideBySideView::renderText
//

void TextDiff::SideBySideView::renderText(const Diff& diff) {
	// setup rendering
	auto drawList = ImGui::GetWindowDrawList();
	auto yTop = drawList->GetClipRectMin().y;
	auto yBottom = drawList->GetClipRectMax().y;

	if (maxColumns * glyphSize.x > textColumnWidth) {
		yBottom -= ImGui::GetStyle().ScrollbarSize + ImGui::GetCurrentWindow()->WindowBorderSize;
	}

	// render left text
	drawList->PushClipRect(ImVec2(leftTextPos, yTop), ImVec2(rightLineNumberPos, yBottom), false);

	for (auto i = firstVisibleRow; i <= lastVisibleRow; i++) {
		const auto& row = rows[i];
		auto y = cursorScreenPos.y + i * glyphSize.y;

		switch(row.type) {
			case DiffType::common:
			case DiffType::deleted:
				renderLine(leftTextPos, y, diff.leftDocument[row.leftLine], row.leftSection, diff);
				break;

			case DiffType::added:
				break;
		}
	}

	drawList->PopClipRect();

	// render right text
	drawList->PushClipRect(ImVec2(rightTextPos, yTop), ImVec2(rightTextEnd, yBottom), false);

	for (auto i = firstVisibleRow; i <= lastVisibleRow; i++) {
		const auto& row = rows[i];
		auto y = cursorScreenPos.y + i * glyphSize.y;

		switch(row.type) {
			case DiffType::common:
			case DiffType::added:
				renderLine(rightTextPos, y, diff.rightDocument[row.rightLine], row.rightSection, diff);
				break;

			case DiffType::deleted:
				break;
		}
	}

	drawList->PopClipRect();
}


//
//	TextDiff::SideBySideView::renderLine
//

void TextDiff::SideBySideView::renderLine(float x, float y, const TextEditor::Line& line, size_t sectionNo, const Diff& diff) {
	// draw colored glyphs for specified line
	auto drawList = ImGui::GetWindowDrawList();

	// determine visible boundaries for this row
	size_t index;
	size_t column;
	size_t endColumn;

	if (diff.config.wordWrap && line.sections) {
		const auto& section = line.sections->at(sectionNo);
		index = section.startIndex;
		column = section.indent;
		endColumn = section.columns;

	} else {
		index = 0;
		column = 0;
		endColumn = line.columns;
	}

	// only process all visible columns
	while (column < endColumn && column <= lastVisibleColumn) {
		auto& glyph = line[index++];
		auto codepoint = glyph.codepoint;
		ImVec2 glyphPos(x + column * glyphSize.x - textScroll, y);

		// handle tabs
		if (codepoint == '\t') {
			if (diff.config.showTabs && column >= firstRenderableColumn) {
				const auto x1 = glyphPos.x + glyphSize.x * 0.3f;
				const auto y1 = glyphPos.y + fontSize * 0.5f;
				const auto x2 = glyphPos.x + glyphSize.x;

				ImVec2 p1, p2, p3, p4;
				p1 = ImVec2(x1, y1);
				p2 = ImVec2(x2, y1);
				p3 = ImVec2(x2 - fontSize * 0.16f, y1 - fontSize * 0.16f);
				p4 = ImVec2(x2 - fontSize * 0.16f, y1 + fontSize * 0.16f);

				drawList->AddLine(p1, p2, diff.palette.get(TextEditor::Color::whitespace));
				drawList->AddLine(p2, p3, diff.palette.get(TextEditor::Color::whitespace));
				drawList->AddLine(p2, p4, diff.palette.get(TextEditor::Color::whitespace));
			}

			column += diff.config.tabSize - (column % diff.config.tabSize);

		// handle spaces
		} else if (codepoint == ' ') {
			if (diff.config.showSpaces && column >= firstRenderableColumn) {
				const auto x1 = glyphPos.x + glyphSize.x * 0.5f;
				const auto y1 = glyphPos.y + fontSize * 0.5f;
				drawList->AddCircleFilled(ImVec2(x1, y1), 1.5f, diff.palette.get(TextEditor::Color::whitespace), 4);
			}

			column++;

		// handle regular glyphs
		} else {
			if (column >= firstRenderableColumn) {
				font->RenderChar(drawList, fontSize, glyphPos, diff.palette.get(glyph.color), codepoint);
			}

			column++;
		}
	}
}


//
//	TextDiff::SideBySideView::renderScrollbars
//

void TextDiff::SideBySideView::renderScrollbars() {
	auto maxColumnsWidth = maxColumns * glyphSize.x;

	if (maxColumnsWidth > textColumnWidth) {
		const ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImRect outerRect = window->Rect();
		auto borderSize = std::round(window->WindowBorderSize * 0.5f);
		auto scrollbarSize = ImGui::GetStyle().ScrollbarSize;

		auto scrollbarTop = std::max(outerRect.Min.y + borderSize, outerRect.Max.y - borderSize - scrollbarSize);
		ImRect leftScrollbarFrame(leftTextPos, scrollbarTop, rightLineNumberPos, scrollbarTop + scrollbarSize);
		ImRect rightScrollbarFrame(rightTextPos, scrollbarTop, rightTextEnd, scrollbarTop + scrollbarSize);
		ImS64 scroll = static_cast<ImS64>(textScroll);

		if (ImGui::ScrollbarEx(
			leftScrollbarFrame,
			ImGui::GetID("leftTextScroll"),
			ImGuiAxis_X,
			&scroll,
			static_cast<ImS64>(textColumnWidth),
			static_cast<ImS64>(maxColumnsWidth),
			ImDrawFlags_RoundCornersAll)) {

			textScroll = static_cast<float>(scroll);
		}

		if (ImGui::ScrollbarEx(
			rightScrollbarFrame,
			ImGui::GetID("rightTextScroll"),
			ImGuiAxis_X,
			&scroll,
			static_cast<ImS64>(textColumnWidth),
			static_cast<ImS64>(maxColumnsWidth),
			ImDrawFlags_RoundCornersAll)) {

			textScroll = static_cast<float>(scroll);
		}

		if (ImGui::IsWindowFocused()) {
			textScroll = std::clamp(textScroll - ImGui::GetIO().MouseWheelH * ImGui::GetFontSize(), 0.0f, maxColumnsWidth - textColumnWidth);

			if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
				textScroll = std::max(textScroll - glyphSize.x, 0.0f);

			} else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
				textScroll = std::min(textScroll + glyphSize.x, maxColumnsWidth - textColumnWidth);

			} else if (ImGui::IsKeyPressed(ImGuiKey_Home)) {
				textScroll = 0.0f;

			} else if (ImGui::IsKeyPressed(ImGuiKey_End)) {
				textScroll = maxColumnsWidth - textColumnWidth;
			}
		}

	} else {
		// no scrolling if the text fits
		textScroll = 0.0f;
	}

	if (ImGui::IsWindowFocused()) {
		if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
			if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
				ImGui::SetScrollY(0.0f);

			} else {
				ImGui::SetScrollY(std::max(ImGui::GetScrollY() - glyphSize.y, 0.0f));
			}

		} else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
			if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
				ImGui::SetScrollY(ImGui::GetScrollMaxY());

			} else {
				ImGui::SetScrollY(std::min(ImGui::GetScrollY() + glyphSize.y, ImGui::GetScrollMaxY()));
			}

		} else if (ImGui::IsKeyPressed(ImGuiKey_PageUp)) {
			ImGui::SetScrollY(std::max(ImGui::GetScrollY() - (visibleRows - 2) * glyphSize.y, 0.0f));

		} else if (ImGui::IsKeyPressed(ImGuiKey_PageDown)) {
			ImGui::SetScrollY(std::min(ImGui::GetScrollY() + (visibleRows - 2) * glyphSize.y, ImGui::GetScrollMaxY()));
		}
	}
}


//
//	TextDiff::SideBySideView::renderMiniMap
//

void TextDiff::SideBySideView::renderMiniMap(const Diff& diff) {
	// based on https://github.com/ocornut/imgui/issues/3114
	if (diff.config.showScrollbarMiniMap) {
		auto window = ImGui::GetCurrentWindow();

		if (window->ScrollbarY) {
			auto drawList = ImGui::GetWindowDrawList();
			auto rect = ImGui::GetWindowScrollbarRect(window, ImGuiAxis_Y);
			auto rowHeight = rect.GetHeight() / static_cast<float>(rows.size());
			auto offset = (rect.Max.x - rect.Min.x) * 0.3f;
			auto left = rect.Min.x + offset;
			auto right = rect.Max.x - offset;

			drawList->PushClipRect(rect.Min, rect.Max, false);

			// render diff locations
			for (size_t i = 0; i < rows.size(); i++) {
				const auto& row = rows[i];

				if (row.type != DiffType::common) {
					auto color = (row.type == DiffType::added) ? diff.addedColor : diff.deletedColor;
					auto ly = std::round(rect.Min.y + i * rowHeight);
					drawList->AddRectFilled(ImVec2(left, ly), ImVec2(right, ly + rowHeight), color);
				}
			}

			drawList->PopClipRect();
		}
	}
}


//
//	TextDiff::SideBySideView::updateLayout
//

void TextDiff::SideBySideView::updateLayout(Diff& diff) {
	rows.clear();
	maxColumns = 0;

	for (auto& lineState : diff.state) {
		switch (lineState.type) {
			case DiffType::common: {
				auto& line = diff.leftDocument[lineState.leftLine];

				for (size_t i = 0; i < line.rows; i++) {
					rows.emplace_back(lineState.type, lineState.leftLine, i, lineState.rightLine, i, line.columns);
				}

				maxColumns = std::max(maxColumns, line.columns);
				break;
			}

			case DiffType::added: {
				auto& line = diff.rightDocument[lineState.rightLine];

				for (size_t i = 0; i < line.rows; i++) {
					rows.emplace_back(lineState.type, 0, 0, lineState.rightLine, i, line.columns);
				}

				maxColumns = std::max(maxColumns, line.columns);
				break;
			}

			case DiffType::deleted: {
				auto& line = diff.leftDocument[lineState.leftLine];

				for (size_t i = 0; i < line.rows; i++) {
					rows.emplace_back(lineState.type, lineState.leftLine, i, 0, 0, line.columns);
				}

				maxColumns = std::max(maxColumns, line.columns);
				break;
			}
		}
	}
}


//
//	TextDiff::splitLines
//

void TextDiff::splitLines(std::vector<std::string_view>& result, const std::string_view& text) {
	size_t prev = TextEditor::CodePoint::skipBOM(text.begin(), text.end()) - text.begin();
	size_t pos;

	while ((pos = text.find('\n', prev)) != std::string_view::npos) {
		result.push_back(text.substr(prev, pos - prev));
		prev = pos + 1;
	}

	result.push_back(text.substr(prev));
}


//
//	TextDiff::updatePalette
//

void TextDiff::updatePalette() {
	// Update palette with the current alpha from style
	diff.paletteAlpha = ImGui::GetStyle().Alpha;

	for (size_t i = 0; i < static_cast<size_t>(TextEditor::Color::count); i++) {
		auto color = ImGui::ColorConvertU32ToFloat4(diff.paletteBase[i]);
		color.w *= diff.paletteAlpha;
		diff.palette[i] = ImGui::ColorConvertFloat4ToU32(color);
	}
}
