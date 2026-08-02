//	TextEditor - A syntax highlighting text editor for ImGui
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <cstdio>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifdef APIENTRY
#undef APIENTRY
#endif
#include <shlobj.h>
#include <windows.h>
#else
#include <cstdlib>
#endif

#include "imgui.h"
#include "ImGuiFileDialog.h"

#include "editor.h"


//
//	Constants
//

#if __APPLE__
#define SHORTCUT "Cmd-"
#else
#define SHORTCUT "Ctrl-"
#endif

static const char* demo =
"// Demo C++ Code\n"
"\n"
"#include <iostream>\n"
"#include <random>\n"
"#include <vector>\n"
"\n"
"int main(int, char**) {\n"
"	std::random_device rd;\n"
"	std::mt19937 gen(rd());\n"
"	std::uniform_int_distribution<> distrib(0, 1000);\n"
"	std::vector<int> numbers;\n"
"\n"
"	for (auto i = 0; i < 100; i++) {\n"
"		numbers.emplace_back(distrib(gen));\n"
"	}\n"
"\n"
"	for (auto n : numbers) {\n"
"		std::cout << n << std::endl;\n"
"	}\n"
"\n"
"	return 0;\n"
"}\n";


//
//	getDocumentDirectory
//

static std::filesystem::path getDocumentDirectory() {
#ifdef _WIN32
	PWSTR buffer;

	if (SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &buffer) == S_OK) {
		std::filesystem::path path(buffer);
		CoTaskMemFree(buffer);
		return path;
	}

#else
	const char* home = std::getenv("HOME");

	if (home) {
		auto path = std::filesystem::path(home) / "Documents";

		if (std::filesystem::exists(path)) {
			return std::filesystem::path(home) / "Documents";
		}
	}

#endif

	return std::filesystem::current_path();
}


//
//	Editor::Editor
//

Editor::Editor() {
	// setup text editor with demo text
	originalText = demo;
	editor.SetText(demo);
	version = editor.GetUndoIndex();
	filename = "untitled.cpp";

	setLanguageByExtention(filename);
	editor.SetShowMiniMapEnabled(true);

	// configure line breaker
	lineBreakConfig.lb2 = false;
	lineBreakConfig.lb3 = false;
	lineBreakConfig.lb4 = false;
	lineBreakConfig.lb5 = false;
	lineBreakConfig.lb6 = false;
	lineBreakConfig.lb13 = false;
	lineBreakConfig.lb14 = false;
	lineBreakConfig.lb15d = false;
	lineBreakConfig.lb19a = false;
	lineBreakConfig.lb21a = false;
	lineBreakConfig.lb21b = false;
	lineBreakConfig.lb26 = false;
	lineBreakConfig.lb27 = false;
	lineBreakConfig.lb28a = false;
	lineBreakConfig.lb29 = false;
	lineBreakConfig.lb30 = false;
	lineBreakConfig.lb30a = false;
	lineBreakConfig.lb30b = false;

	try {
		std::filesystem::current_path(getDocumentDirectory());

	} catch (std::exception&) {
	}
}


//
//	Editor::newFile
//

void Editor::newFile() {
	if (isDirty()) {
		showConfirmClose([this]() {
			originalText.clear();
			editor.SetText("");
			version = editor.GetUndoIndex();
			filename = "untitled";
			setLanguageByExtention(filename);
		});

	} else {
		if (lsp.IsRunning()) {
			lsp.CloseDocument(filename);
		}

		originalText.clear();
		editor.SetText("");
		version = editor.GetUndoIndex();
		filename = "untitled";
		setLanguageByExtention(filename);
	}
}


//
//	Editor::openFile
//

void Editor::openFile() {
	if (isDirty()) {
		showConfirmClose([this]() {
			showFileOpen();
		});

	} else {
		showFileOpen();
	}
}

void Editor::openFile(const std::string& path) {
	try {
		if (lsp.IsRunning()) {
			lsp.CloseDocument(filename);
		}

		std::ifstream stream(path.c_str());
		std::string text;

		stream.seekg(0, std::ios::end);
		text.reserve(stream.tellg());
		stream.seekg(0, std::ios::beg);

		text.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
		stream.close();

		originalText = text;
		editor.SetText(text);
		version = editor.GetUndoIndex();
		filename = path;

		setLanguageByExtention(filename);

	} catch (std::exception& e) {
		showError(e.what());
	}
}


//
//	Editor::saveFile
//

void Editor::saveFile() {
	try {
		editor.StripTrailingWhitespaces();
		std::ofstream stream(filename.c_str());
		stream << editor.GetText();
		stream.close();
		version = editor.GetUndoIndex();

	} catch (std::exception& e) {
		showError(e.what());
	}
}


//
//	Editor::render
//

void Editor::render() {
	// create the outer window
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar);

	// add a menubar
	renderMenuBar();

	// support language server bridge
	if (lsp.IsRunning()) {
		lsp.Update(filename);
	}

	// render the text editor widget
	auto area = ImGui::GetContentRegionAvail();
	auto& style = ImGui::GetStyle();
	auto statusBarHeight = ImGui::GetFrameHeight() + 2.0f * style.WindowPadding.y;
	auto editorSize = ImVec2(0.0f, area.y - style.ItemSpacing.y - statusBarHeight);
	ImGui::PushFont(nullptr, fontSize);
	editor.Render("TextEditor", editorSize);
	ImGui::PopFont();

	// render a statusbar
	ImGui::Spacing();
	renderStatusBar();

	if (state == State::diff) {
		renderDiff();

	} else if (state == State::openFile) {
		renderFileOpen();

	} else if (state == State::saveFileAs) {
		renderSaveAs();

	} else if (state == State::confirmClose) {
		renderConfirmClose();

	} else if (state == State::confirmQuit) {
		renderConfirmQuit();

	} else if (state == State::confirmError) {
		renderConfirmError();

	} else if (state == State::addSquiggle) {
		renderAddSquiggle();

	} else if (state == State::clearSquiggles) {
		renderClearSquiggle();
	}

	ImGui::End();
	ImGui::PopStyleVar();

	// render debug information (if required)
	if (showDebugInformation) {
		renderDebugInformation();
	}

	// render notifications
	auto mainWindowSize = ImGui::GetMainViewport()->Size;
	auto mainWindowPos = ImGui::GetMainViewport()->Pos;
	float offset = statusBarHeight + style.ItemSpacing.y * 2.0f;

	notifications.Render(ImVec2(
		mainWindowPos.x + mainWindowSize.x - ImGui::GetStyle().ItemSpacing.x,
		mainWindowPos.y + mainWindowSize.y - ImGui::GetStyle().ItemSpacing.y - offset));
}


//
//	Editor::tryToQuit
//

void Editor::tryToQuit() {
	if (isDirty()) {
		showConfirmQuit();

	} else {
		done = true;
	}
}


//
//	Editor::renderMenuBar
//

void Editor::renderMenuBar() {
	// create menubar
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New", SHORTCUT "N")) { newFile(); }
			if (ImGui::MenuItem("Open...", SHORTCUT "O")) { openFile(); }

			ImGui::Separator();
			if (ImGui::MenuItem("Save", SHORTCUT "S", nullptr, isSavable())) { saveFile(); }
			if (ImGui::MenuItem("Save As...")) { showSaveFileAs(); }

			ImGui::Separator();
			if (ImGui::MenuItem("Quit", SHORTCUT "Q")) { tryToQuit(); }

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo", " " SHORTCUT "Z", nullptr, editor.CanUndo())) { editor.Undo(); }
#if __APPLE__
			if (ImGui::MenuItem("Redo", "^" SHORTCUT "Z", nullptr, editor.CanRedo())) { editor.Redo(); }
#else
			if (ImGui::MenuItem("Redo", " " SHORTCUT "Y", nullptr, editor.CanRedo())) { editor.Redo(); }
#endif

			ImGui::Separator();
			if (ImGui::MenuItem("Cut", " " SHORTCUT "X", nullptr, editor.AnyCursorHasSelection())) { editor.Cut(); }
			if (ImGui::MenuItem("Copy", " " SHORTCUT "C", nullptr, editor.AnyCursorHasSelection())) { editor.Copy(); }
			if (ImGui::MenuItem("Paste", " " SHORTCUT "V", nullptr, ImGui::GetClipboardText() != nullptr)) { editor.Paste(); }

			ImGui::Separator();
			bool flag;
			flag = editor.IsInsertSpacesOnTabs(); if (ImGui::MenuItem("Insert Spaces on Tabs", nullptr, &flag)) { editor.SetInsertSpacesOnTabs(flag); };

			if (ImGui::MenuItem("Tabs To Spaces")) { editor.TabsToSpaces(); }
			if (ImGui::MenuItem("Spaces To Tabs", nullptr, nullptr, !editor.IsInsertSpacesOnTabs())) { editor.SpacesToTabs(); }
			if (ImGui::MenuItem("Strip Trailing Whitespaces")) { editor.StripTrailingWhitespaces(); }

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Selection")) {
			if (ImGui::MenuItem("Select All", " " SHORTCUT "A", nullptr, !editor.IsEmpty())) { editor.SelectAll(); }

			ImGui::Separator();
			if (ImGui::MenuItem("Indent Line(s)", " " SHORTCUT "]", nullptr, !editor.IsEmpty())) { editor.IndentLines(); }
			if (ImGui::MenuItem("Deindent Line(s)", " " SHORTCUT "[", nullptr, !editor.IsEmpty())) { editor.DeindentLines(); }
			if (ImGui::MenuItem("Move Line(s) Up", nullptr, nullptr, !editor.IsEmpty())) { editor.MoveUpLines(); }
			if (ImGui::MenuItem("Move Line(s) Down", nullptr, nullptr, !editor.IsEmpty())) { editor.MoveDownLines(); }
			if (ImGui::MenuItem("Toggle Comments", " " SHORTCUT "/", nullptr, editor.HasLanguage())) { editor.ToggleComments(); }

			ImGui::Separator();
			if (ImGui::MenuItem("To Uppercase", nullptr, nullptr, editor.AnyCursorHasSelection())) { editor.SelectionToUpperCase(); }
			if (ImGui::MenuItem("To Lowercase", nullptr, nullptr, editor.AnyCursorHasSelection())) { editor.SelectionToLowerCase(); }

			ImGui::Separator();
			if (ImGui::MenuItem("Add Next Occurrence", " " SHORTCUT "D", nullptr, editor.CurrentCursorHasSelection())) { editor.AddNextOccurrence(); }
			if (ImGui::MenuItem("Select All Occurrences", "^" SHORTCUT "D", nullptr, editor.CurrentCursorHasSelection())) { editor.SelectAllOccurrences(); }

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Reset Zoom", " " SHORTCUT "0")) { resetFontSize(); }
			if (ImGui::MenuItem("Zoom In", " " SHORTCUT "+")) { increaseFontSize(); }
			if (ImGui::MenuItem("Zoom Out", " " SHORTCUT "-")) { decreaseFontSize(); }

			ImGui::Separator();
			if (ImGui::MenuItem("Show Diff", " " SHORTCUT "I")) { showDiff(); }

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Find")) {
			if (ImGui::MenuItem("Find", " " SHORTCUT "F")) { editor.OpenFindReplaceWindow(); }
			if (ImGui::MenuItem("Find Next", " " SHORTCUT "G", nullptr, editor.HasFindString())) { editor.FindNext(); }
			if (ImGui::MenuItem("Find All", "^" SHORTCUT "G", nullptr, editor.HasFindString())) { editor.FindAll(); }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options")) {
			if (ImGui::BeginMenu("Tab Size")) {
				if (ImGui::MenuItem("1")) { editor.SetTabSize(1); }
				if (ImGui::MenuItem("2")) { editor.SetTabSize(2); }
				if (ImGui::MenuItem("4")) { editor.SetTabSize(4); }
				if (ImGui::MenuItem("8")) { editor.SetTabSize(8); }
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Line Spacing")) {
				if (ImGui::MenuItem("1.0")) { editor.SetLineSpacing(1.0f); }
				if (ImGui::MenuItem("1.25")) { editor.SetLineSpacing(1.25f); }
				if (ImGui::MenuItem("1.5")) { editor.SetLineSpacing(1.5f); }
				if (ImGui::MenuItem("1.75")) { editor.SetLineSpacing(1.75f); }
				if (ImGui::MenuItem("2.0")) { editor.SetLineSpacing(2.0f); }
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("MiniMap Columns")) {
				int width = static_cast<int>(editor.GetMiniMapColumns());
				if (ImGui::SliderInt("##miniMapColumns", &width, 0, 200)) { editor.SetMiniMapColumns(static_cast<size_t>(width)); }
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Color Palette")) {
				if (ImGui::MenuItem("Dark")) { setDarkPalette(); }
				if (ImGui::MenuItem("Light")) { setLightPalette(); }
				ImGui::EndMenu();
			}

			ImGui::Separator();

			bool flag;
			flag = editor.IsReadOnlyEnabled(); if (ImGui::MenuItem("Read Only", nullptr, &flag)) { editor.SetReadOnlyEnabled(flag); };
			flag = editor.IsCaretsVisible(); if (ImGui::MenuItem("Carets Visible", nullptr, &flag)) { editor.SetCaretsVisible(flag); };
			flag = editor.IsOverwriteEnabled(); if (ImGui::MenuItem("Overwrite", nullptr, &flag)) { editor.SetOverwriteEnabled(flag); };
			flag = editor.IsWordWrapEnabled(); if (ImGui::MenuItem("Word Wrap", nullptr, &flag)) { editor.SetWordWrapEnabled(flag); };
			flag = editor.IsLineFoldingEnabled(); if (ImGui::MenuItem("Line Folding", nullptr, &flag)) { editor.SetLineFoldingEnabled(flag); };
			flag = editor.IsShowWhitespacesEnabled(); if (ImGui::MenuItem("Show Whitespaces", nullptr, &flag)) { editor.SetShowWhitespacesEnabled(flag); };
			flag = editor.IsShowSpacesEnabled(); if (ImGui::MenuItem("Show Spaces", nullptr, &flag)) { editor.SetShowSpacesEnabled(flag); };
			flag = editor.IsShowTabsEnabled(); if (ImGui::MenuItem("Show Tabs", nullptr, &flag)) { editor.SetShowTabsEnabled(flag); };
			flag = editor.IsShowLineNumbersEnabled(); if (ImGui::MenuItem("Show Line Numbers", nullptr, &flag)) { editor.SetShowLineNumbersEnabled(flag); };
			flag = editor.IsShowingMatchingBrackets(); if (ImGui::MenuItem("Show Matching Brackets", nullptr, &flag)) { editor.SetShowMatchingBrackets(flag); };
			flag = editor.IsCompletingPairedGlyphs(); if (ImGui::MenuItem("Complete Matching Glyphs", nullptr, &flag)) { editor.SetCompletePairedGlyphs(flag); };
			flag = editor.IsShowMiniMapEnabled(); if (ImGui::MenuItem("Show Mini Map", nullptr, &flag)) { editor.SetShowMiniMapEnabled(flag); };
			flag = editor.IsShowScrollbarMiniMapEnabled(); if (ImGui::MenuItem("Show Scrollbar Mini Map", nullptr, &flag)) { editor.SetShowScrollbarMiniMapEnabled(flag); };
			flag = editor.IsShowPanScrollIndicatorEnabled(); if (ImGui::MenuItem("Show Pan/Scroll Indicator", nullptr, &flag)) { editor.SetShowPanScrollIndicatorEnabled(flag); };
			flag = editor.IsMiddleMousePanMode(); if (ImGui::MenuItem("Middle Mouse Pan Mode", nullptr, &flag)) { if (flag) editor.SetMiddleMousePanMode(); else editor.SetMiddleMouseScrollMode(); };

			ImGui::Separator();

			if (ImGui::BeginMenu("Line Number Left Margin")) {
				int margin = static_cast<int>(editor.GetLineNumberLeftMargin());
				if (ImGui::SliderInt("##margin", &margin, 0, 4)) { editor.SetLineNumberLeftMargin(static_cast<size_t>(margin)); }
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Decoration Left Margin")) {
				int margin = static_cast<int>(editor.GetDecorationLeftMargin());
				if (ImGui::SliderInt("##margin", &margin, 0, 4)) { editor.SetDecorationLeftMargin(static_cast<size_t>(margin)); }
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Text Left Margin")) {
				int margin = static_cast<int>(editor.GetTextLeftMargin());
				if (ImGui::SliderInt("##margin", &margin, 0, 4)) { editor.SetTextLeftMargin(static_cast<size_t>(margin)); }
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Examples")) {
			if (ImGui::MenuItem("Trie-based AutoComplete", nullptr, &demoTrieAutoComplete)) { toggleTrieAutoComplete(); }
			if (ImGui::MenuItem("Language Server Protocol Bridge", nullptr, &demoLspBridge)) { toggleLspBridge(); }
			if (ImGui::MenuItem("Show Word at Mouse", nullptr, &showWordAtMouse)) { toggleShowWordAtMouse(); }
			if (ImGui::MenuItem("Show Line Markers", nullptr, &showLineMarkers)) { toggleLineMarkers(); }
			if (ImGui::MenuItem("Show Line Decorator", nullptr, &showLineDecorator)) { toggleLineDecorator(); }
			if (ImGui::MenuItem("Show Context Menus", nullptr, &showContextMenus)) { toggleContextMenus(); }
			if (ImGui::MenuItem("Enable Unicode Line Break Algorithm", nullptr, &enableUnicodeLineBreakAlgorithm)) { toggleLineBreak(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Add Squiggle(s)", nullptr, nullptr, editor.AnyCursorHasSelection())) { showAddSquiggle(); }
			if (ImGui::MenuItem("Clear Squiggles", nullptr, nullptr, editor.HasSquiggles())) { clearSquiggles(); }
			if (ImGui::MenuItem("Clear Squiggles by Type", nullptr, nullptr, editor.HasSquiggles())) { showClearSquiggles(); }
			ImGui::Separator();
			bool navigationMode = ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard;
			if (ImGui::MenuItem("Keyboard/Gamepad Navigation Mode", " " SHORTCUT "Alt-N", &navigationMode)) { toggleNavigationMode(); }
			ImGui::MenuItem("Show Debug Information", nullptr, &showDebugInformation);
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	// handle keyboard shortcuts
	if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_N)) { newFile(); }
	else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O)) { openFile(); }
	else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S)) { if (filename == "untitled") { showSaveFileAs(); } else { saveFile(); } }
	else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_I)) { showDiff(); }
	else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_0)) { resetFontSize(); }
	else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Equal)) { increaseFontSize(); }
	else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Minus)) { decreaseFontSize(); }
	else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Alt | ImGuiKey_N, ImGuiInputFlags_RouteAlways)) { toggleNavigationMode(); }
}


//
//	Editor::renderStatusBar
//

void Editor::renderStatusBar() {
	// language support
	static const char* languages[] = {"None", "C", "C++", "Cs", "AngelScript", "Lua", "Python", "GLSL", "HLSL",  "JSON", "Markdown", "SQL"};
	std::string language = editor.GetLanguageName();

	// create a statusbar window
	ImGui::PushStyleColor(ImGuiCol_ChildBg, editor.GetPalette().get(TextEditor::Color::background));
	ImGui::BeginChild("StatusBar", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
	ImGui::SetNextItemWidth(120.0f);

	// allow user to select language for colorizing
	if (ImGui::BeginCombo("##LanguageSelector", language.c_str())) {
		for (int n = 0; n < IM_ARRAYSIZE(languages); n++) {
			bool selected = (language == languages[n]);

			if (ImGui::Selectable(languages[n], selected)) {
				setLanguageByName(languages[n]);
			}

			if (selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}

	// support show word at mouse
	char word[32];

	if (showWordAtMouse) {
		auto wordAtMousePos = editor.GetWordAtMousePos(ImGui::GetMousePos());

		if (wordAtMousePos.size()) {
			std::snprintf(word, sizeof(word), "Word: %s ", wordAtMousePos.c_str());

		} else {
			word[0] = 0;
		}

	} else {
		word[0] = 0;
	}

	// determine status message
	auto tabSize = editor.GetTabSize();
	auto cursorPos = editor.DocPos2VisPos(editor.GetCurrentCursorPosition());
	auto fn = std::filesystem::path(filename).filename().string();
	char status[256];

	std::snprintf(
		status,
		sizeof(status),
		"%sLn %zu, Col %zu  Tab Size: %zu  File: %s",
		word,
		cursorPos.row + 1,
		cursorPos.column + 1,
		tabSize,
		fn.c_str()
	);

	// determine horizontal gap so the rest is right aligned
	ImGui::SameLine(0.0f, 0.0f);
	auto availableSpace = ImGui::GetContentRegionAvail().x;
	auto messageWidth = ImGui::CalcTextSize(status).x;
	auto dirtyWidth = ImGui::CalcTextSize("#").x * 3.0f;
	ImGui::SameLine(0.0f, availableSpace - messageWidth - dirtyWidth);

	// render status text
	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted(status);

	// render "text dirty" indicator
	ImGui::SameLine(0.0f, ImGui::CalcTextSize("#").x * 1.0f);
	auto drawlist = ImGui::GetWindowDrawList();
	auto pos = ImGui::GetCursorScreenPos();
	auto offset = ImGui::GetFrameHeight() * 0.5f;
	auto radius = offset * 0.6f;
	auto color = isDirty() ? IM_COL32(164, 0, 0, 255) : IM_COL32(164, 164, 164, 255);
	drawlist->AddCircleFilled(ImVec2(pos.x + offset, pos.y + offset), radius, color);

	ImGui::EndChild();
	ImGui::PopStyleColor();
}


//
//	Editor::showDiff
//

void Editor::showDiff() {
	diff.SetText(originalText, editor.GetText());
	state = State::diff;
	popup = true;
}


//
//	Editor::showFileOpen
//

void Editor::showFileOpen() {
	// open a file selector dialog
	IGFD::FileDialogConfig config;
	config.countSelectionMax = 1;

	config.flags =
		ImGuiFileDialogFlags_Modal |
		ImGuiFileDialogFlags_DontShowHiddenFiles |
		ImGuiFileDialogFlags_ReadOnlyFileNameField;

	ImGuiFileDialog::Instance()->OpenDialog("file-open", "Select File to Open...", ".*", config);
	state = State::openFile;
}


//
//	Editor::showSaveFileAs
//

void Editor::showSaveFileAs() {
	IGFD::FileDialogConfig config;
	config.countSelectionMax = 1;

	config.flags =
		ImGuiFileDialogFlags_Modal |
		ImGuiFileDialogFlags_DontShowHiddenFiles |
		ImGuiFileDialogFlags_ConfirmOverwrite;

	ImGuiFileDialog::Instance()->OpenDialog("file-saveas", "Save File as...", "*", config);
	state = State::saveFileAs;
}


//
//	Editor::showConfirmClose
//

void Editor::showConfirmClose(std::function<void()> callback) {
	onConfirmClose = callback;
	state = State::confirmClose;
	popup = true;
}


//
//	Editor::showConfirmQuit
//

void Editor::showConfirmQuit() {
	state = State::confirmQuit;
	popup = true;
}


//
//	Editor::showError
//

void Editor::showError(const std::string &message) {
	errorMessage = message;
	state = State::confirmError;
	popup = true;
}


//
//	Editor::showAddSquiggle
//

void Editor::showAddSquiggle() {
	state = State::addSquiggle;
	popup = true;
}


//
//	Editor::showClearSquiggles
//

void Editor::showClearSquiggles() {
	state = State::clearSquiggles;
	popup = true;
}


//
//	Editor::renderDiff
//

void Editor::renderDiff() {
	const auto viewport = ImGui::GetMainViewport();

	if (popup) {
		ImGui::OpenPopup("Changes since Opening File##diff");
		ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		diff.SetFocus();
		popup = false;
	}

	if (ImGui::BeginPopupModal("Changes since Opening File##diff", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		diff.Render("diff", viewport->Size * 0.8f, ImGuiChildFlags_Borders);

		ImGui::Separator();
		static constexpr float buttonWidth = 80.0f;
		auto buttonOffset = ImGui::GetContentRegionAvail().x - buttonWidth;
		bool sideBySide = diff.GetSideBySideMode();
		bool wordWrap = diff.IsWordWrapEnabled();

		if (ImGui::Checkbox("Side-by-Side", &sideBySide)) {
			diff.SetSideBySideMode(sideBySide);
		}

		ImGui::SameLine();

		if (ImGui::Checkbox("Word Wrap", &wordWrap)) {
			diff.SetWordWrapEnabled(wordWrap);
		}

		ImGui::SameLine();
		ImGui::Indent(buttonOffset);

		if (ImGui::Button("OK", ImVec2(buttonWidth, 0.0f)) || ImGui::Shortcut(ImGuiKey_Escape)) {
			ImGui::CloseCurrentPopup();
			state = State::edit;
		}

		ImGui::EndPopup();
	}
}


//
//	Editor::renderFileOpen
//

void Editor::renderFileOpen() {
	// handle file open dialog
	ImVec2 maxSize = ImGui::GetMainViewport()->Size;
	ImVec2 minSize = maxSize * 0.5f;
	auto dialog = ImGuiFileDialog::Instance();

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (dialog->Display("file-open", ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
		// open selected file (if required)
		if (dialog->IsOk()) {
			openFile(dialog->GetFilePathName());
			state = State::edit;
		}

		// close dialog
		dialog->Close();
	}
}


//
//	Editor::renderSaveAs
//

void Editor::renderSaveAs() {
	// handle saveas dialog
	ImVec2 maxSize = ImGui::GetMainViewport()->Size;
	ImVec2 minSize = maxSize * 0.5f;
	auto dialog = ImGuiFileDialog::Instance();

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (dialog->Display("file-saveas", ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
		// open selected file if required
		if (dialog->IsOk()) {
			filename = dialog->GetFilePathName();
			saveFile();
			state = State::edit;

		} else {
			state = State::edit;
		}

		// close dialog
		dialog->Close();
	}
}


//
//	Editor::renderConfirmClose
//

void Editor::renderConfirmClose() {
	if (popup) {
		ImGui::OpenPopup("Confirm Close");
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		popup = false;
	}

	if (ImGui::BeginPopupModal("Confirm Close", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("This file has changed!\nDo you really want to delete it?\n\n");
		ImGui::Separator();

		static constexpr float buttonWidth = 80.0f;
		ImGui::Indent(ImGui::GetContentRegionAvail().x - buttonWidth * 2.0f - ImGui::GetStyle().ItemSpacing.x);

		if (ImGui::Button("OK", ImVec2(buttonWidth, 0.0f))) {
			state = State::edit;
			onConfirmClose();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)) || ImGui::Shortcut(ImGuiKey_Escape)) {
			state = State::edit;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}


//
//	Editor::renderConfirmQuit
//

void Editor::renderConfirmQuit() {
	if (popup) {
		ImGui::OpenPopup("Quit Editor?");
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		popup = false;
	}

	if (ImGui::BeginPopupModal("Quit Editor?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Your text has changed and is not saved!\nDo you really want to quit?\n\n");
		ImGui::Separator();

		static constexpr float buttonWidth = 80.0f;
		ImGui::Indent(ImGui::GetContentRegionAvail().x - buttonWidth * 2.0f - ImGui::GetStyle().ItemSpacing.x);

		if (ImGui::Button("OK", ImVec2(buttonWidth, 0.0f))) {
			done = true;
			state = State::edit;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)) || ImGui::Shortcut(ImGuiKey_Escape)) {
			state = State::edit;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}


//
//	Editor::renderConfirmError
//

void Editor::renderConfirmError() {
	if (popup) {
		ImGui::OpenPopup("Error");
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		popup = false;
	}

	if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("%s\n", errorMessage.c_str());
		ImGui::Separator();

		static constexpr float buttonWidth = 80.0f;
		ImGui::Indent(ImGui::GetContentRegionAvail().x - buttonWidth);

		if (ImGui::Button("OK", ImVec2(buttonWidth, 0.0f)) || ImGui::Shortcut(ImGuiKey_Escape)) {
			errorMessage.clear();
			state = State::edit;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}


//
//	Editor::renderAddSquiggle
//

void Editor::renderAddSquiggle() {
	if (popup) {
		ImGui::OpenPopup("Add Squiggle");
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		popup = false;
	}

	if (ImGui::BeginPopupModal("Add Squiggle", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		auto type = static_cast<int>(squiggleType);
		if (ImGui::SliderInt("Type", &type, 1, 5)) { squiggleType = static_cast<size_t>(type); }
		ImGui::ColorEdit4("Color", (float*) &squiggleColor);
		ImGui::InputText("Tool Tip", squiggleToolTip, sizeof(squiggleToolTip));
		ImGui::Separator();

		static constexpr float buttonWidth = 80.0f;
		ImGui::Indent(ImGui::GetContentRegionAvail().x - buttonWidth * 2.0f - 5.0f);

		if (ImGui::Button("OK", ImVec2(buttonWidth, 0.0f))) {
			ImU32 color = squiggleColor;

			for (size_t i = 0; i < editor.GetNumberOfCursors(); i++) {
				auto selection = editor.GetCursorSelection(i);

				if (selection.start != selection.end) {
					editor.AddSquiggle(selection.start, selection.end, squiggleType, color, squiggleToolTip);
				}
			}

			state = State::edit;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine(0.0f, 5.0f);

		if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)) || ImGui::Shortcut(ImGuiKey_Escape)) {
			state = State::edit;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}


//
//	Editor::renderClearSquiggle
//

void Editor::renderClearSquiggle() {
	if (popup) {
		ImGui::OpenPopup("Clear Squiggles");
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		popup = false;
	}

	if (ImGui::BeginPopupModal("Clear Squiggles", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		auto type = static_cast<int>(squiggleType);
		if (ImGui::SliderInt("Type", &type, 1, 5)) { squiggleType = static_cast<size_t>(type); }
		ImGui::Separator();

		static constexpr float buttonWidth = 80.0f;
		ImGui::Indent(ImGui::GetContentRegionAvail().x - buttonWidth * 2.0f - 5.0f);

		if (ImGui::Button("OK", ImVec2(buttonWidth, 0.0f))) {
			editor.ClearSquiggles(squiggleType);
			state = State::edit;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine(0.0f, 5.0f);

		if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)) || ImGui::Shortcut(ImGuiKey_Escape)) {
			state = State::edit;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}


//
//	renderDebugInformation
//

void Editor::renderDebugInformation() {
	if (showDebugInformation) {
		std::string information;

		if (getBackendDebugInformation) {
			information = getBackendDebugInformation();
		}

		auto& io = ImGui::GetIO();
		auto& style = ImGui::GetStyle();

		information += "Dear ImGui:\n";
		information += std::format("io.DisplaySize: {}, {}\n", io.DisplaySize.x, io.DisplaySize.y);
		information += std::format("io.DisplayFramebufferScale: {}, {}\n", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		information += std::format("style.FontSizeBase: {}\n", style.FontSizeBase);
		information += std::format("style.FontScaleMain: {}\n", style.FontScaleMain);
		information += std::format("style.FontScaleDpi: {}\n", style.FontScaleDpi);
		information += std::format("GetLineHeight: {}\n", editor.GetLineHeight());
		information += std::format("GetGlyphWidth: {}", editor.GetGlyphWidth());

		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoFocusOnAppearing;

		ImGui::Begin("Debug Information", nullptr, flags);
		ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
		ImGui::TextUnformatted(information.c_str());
		ImGui::End();
	}
}


//
//	Editor::setLanguage
//

void Editor::setLanguage(const TextEditor::Language* language) {
	if (editor.GetLanguage() != language) {
		editor.SetLanguage(language);
		diff.SetLanguage(language);

		if (lsp.IsRunning()) {
			lsp.CloseDocument(filename);

			if (language->name == "C++") {
				lsp.OpenDocument(filename, editor, lspOptions);
			}
		}
	}
}


//
//	Editor::setLanguageByName
//

void Editor::setLanguageByName(const std::string& name) {
	if (name == "C") {
		setLanguage(TextEditor::Language::C());

	} else if (name == "C++") {
		setLanguage(TextEditor::Language::Cpp());

	} else if (name == "C#") {
		setLanguage(TextEditor::Language::Cs());

	} else if (name == "AngelScript") {
		setLanguage(TextEditor::Language::AngelScript());

	} else if (name == "Lua") {
		setLanguage(TextEditor::Language::Lua());

	} else if (name == "Python") {
		setLanguage(TextEditor::Language::Python());

	} else if (name == "GLSL") {
		setLanguage(TextEditor::Language::Glsl());

	} else if (name == "HLSL") {
		setLanguage(TextEditor::Language::Hlsl());

	} else if (name == "JSON") {
		setLanguage(TextEditor::Language::Json());

	} else if (name == "Markdown") {
		setLanguage(TextEditor::Language::Markdown());

	} else if (name == "SQL") {
		setLanguage(TextEditor::Language::Sql());

	} else {
		setLanguage(nullptr);
	}
}


//
//	Editor::setLanguageByExtention
//

void Editor::setLanguageByExtention(const std::string& name) {
	std::filesystem::path path(name);
	auto extension = path.extension();

	if (extension == ".cpp" || extension == ".h" || extension == ".hpp") {
		setLanguage(TextEditor::Language::Cpp());

	} else if (extension == ".c") {
		setLanguage(TextEditor::Language::C());

	} else if (extension == ".cs") {
		setLanguage(TextEditor::Language::Cs());

	} else if (extension == ".as") {
		setLanguage(TextEditor::Language::AngelScript());

	} else if (extension == ".lua") {
		setLanguage(TextEditor::Language::Lua());

	} else if (extension == ".py") {
		setLanguage(TextEditor::Language::Python());

	} else if (extension == ".glsl") {
		setLanguage(TextEditor::Language::Glsl());

	} else if (extension == ".hlsl") {
		setLanguage(TextEditor::Language::Hlsl());

	} else if (extension == ".json") {
		setLanguage(TextEditor::Language::Json());

	} else if (extension == ".md") {
		setLanguage(TextEditor::Language::Markdown());

	} else if (extension == ".sql") {
		setLanguage(TextEditor::Language::Sql());

	} else {
		setLanguage(nullptr);
	}
}


//
//	Editor::toggleNavigationMode
//

void Editor::toggleNavigationMode() {
	auto& io = ImGui::GetIO();

	if (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) {
		io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
		notifications.Add(Notifications::Type::info, "Keyboard/gamepad navigation mode deactivated");

	} else {
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		notifications.Add(Notifications::Type::info, "Keyboard/gamepad navigation mode activated");
	}
}


//
//	Editor::toggleTrieAutoComplete
//

void Editor::toggleTrieAutoComplete() {
	// see if we are turning it on or off
	if (demoTrieAutoComplete) {
		// deactivate language server demo (if required)
		if (demoLspBridge) {
			demoLspBridge = false;
			toggleLspBridge();
		}

		// connect autocomplete helper to editor
		trieAutoComplete.Connect(&editor);
		notifications.Add(Notifications::Type::info, "Autocomplete activated");

	} else {
		// disconnect autocomplete helper from editor
		trieAutoComplete.Disconnect();
		notifications.Add(Notifications::Type::info, "Autocomplete deactivated");
	}
}


//
//	Editor::toggleLspBridge
//

void Editor::toggleLspBridge() {
	// see if we are turning it on or off
	if (demoLspBridge) {
		// deactivate trie autocomplete (if required)
		if (demoTrieAutoComplete) {
			demoTrieAutoComplete = false;
			toggleTrieAutoComplete();
		}

		// start the language server
		if (lsp.Start(std::filesystem::current_path().string(), "clangd", {"--log=error"})) {
			notifications.Add(Notifications::Type::info, "Started language server");

			if (editor.GetLanguageName() == "C++") {
				lsp.OpenDocument(filename, editor, lspOptions);
			}

		} else {
			// report possible errors
			notifications.Add(Notifications::Type::error, lsp.GetError(), 6000);
			demoLspBridge = false;
		}

	} else {
		// stop the language server
		lsp.Stop();
		notifications.Add(Notifications::Type::info, "Stopped language server");
	}
}


//
//	Editor::toggleShowWordAtMouse
//

void Editor::toggleShowWordAtMouse() {
	// see if we are turning it on or off
	if (showWordAtMouse) {
		notifications.Add(Notifications::Type::info, "Show word at mouse activated");

	} else {
		notifications.Add(Notifications::Type::info, "Show word at mouse deactivated");
	}
}


//
//	Editor::toggleLineMarkers
//

void Editor::toggleLineMarkers() {
	// see if we are turning it on or off
	if (showLineMarkers) {
		size_t errorlineNumber = 7;
		size_t breakPointLineNumber = 9;
		size_t justBecauseLineNumber = 12;
		editor.AddMarker(errorlineNumber, 0, IM_COL32(128, 0, 32, 128), "", "Error detected on this line");
		editor.AddMarker(breakPointLineNumber, IM_COL32(0, 255, 32, 100), 0, "", "");
		editor.AddMarker(breakPointLineNumber, IM_COL32(0, 255, 32, 100), 0, "", "");
		editor.AddMarker(justBecauseLineNumber, IM_COL32(255, 224, 32, 100), IM_COL32(255, 224, 32, 100), "Just Because", "Just Because");
		notifications.Add(Notifications::Type::info, "Line markers activated");

	} else {
		editor.ClearMarkers();
		notifications.Add(Notifications::Type::info, "Line markers deactivated");
	}
}


//
//	Editor::toggleLineDecorator
//

void Editor::toggleLineDecorator() {
	// see if we are turning it on or off
	if (showLineDecorator) {
		editor.SetLineDecorator(3, [](const TextEditor::Decorator& decorator) {
			if (decorator.line == 10 || decorator.line == 15|| decorator.line == 16) {
				auto size = decorator.height - 1.0f;

				if (decorator.line == 15) {
					if (ImGui::Button("^", ImVec2(size, size))) {
						// performing some useful action
					}

					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("Perform useful action");
					}

				} else {
					ImGui::Dummy(ImVec2(size, size));
				}

				ImGui::SameLine(0.0f, 1.0f);

				auto pos = ImGui::GetCursorScreenPos();
				auto drawlist = ImGui::GetWindowDrawList();

				drawlist->AddCircleFilled(
					ImVec2(pos.x + size * 0.5f, pos.y + size * 0.5f),
					(size - 6.0f) * 0.5f,
					IM_COL32(128, 0, 0, 255));

				ImGui::InvisibleButton("Invisible", ImVec2(size, size));

				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Call Something")) { /* handle click */ }
					if (ImGui::MenuItem("Call Something Else")) { /* handle click */ }
					ImGui::EndPopup();
				}
			}
		});

		notifications.Add(Notifications::Type::info, "line decorator activated");

	} else {
		editor.ClearLineDecorator();
		notifications.Add(Notifications::Type::info, "line decorator deactivated");
	}
}


//
//	Editor::toggleContextMenus
//

void Editor::toggleContextMenus() {
	// see if we are turning it on or off
	if (showContextMenus) {
		editor.SetLineNumberContextMenuCallback([]([[maybe_unused]] TextEditor::PopupData& data) {
			if (ImGui::MenuItem("Set Breakpoint")) { /* handle click */ }
			if (ImGui::MenuItem("Remove Breakpoint")) { /* handle click */ }
		});

		editor.SetTextContextMenuCallback([](const TextEditor::PopupData& data) {
			ImGui::Text("Line %zu, index %zu", data.pos.line + 1, data.pos.index + 1);
		});

		notifications.Add(Notifications::Type::info, "Context menus activated");

	} else {
		editor.ClearLineNumberContextMenuCallback();
		editor.ClearTextContextMenuCallback();
		notifications.Add(Notifications::Type::info, "Context menus deactivated");
	}
}


//
//	Editor::toggleLineBreak
//

void Editor::toggleLineBreak() {
	// see if we are turning it on or off
	if (enableUnicodeLineBreakAlgorithm) {
		notifications.Add(Notifications::Type::info, "Switched line break algorithm to unicode annex 14 mode");

	} else{
		notifications.Add(Notifications::Type::info, "Switched line break algorithm to simple mode");
	}

	lineBreakConfig.useUnicodeAnnex14 = enableUnicodeLineBreakAlgorithm;
	editor.SetWordWrapEnabled(true);
	editor.SetLineBreakConfig(lineBreakConfig);
}


//
//	Editor::clearSquiggles
//

void Editor::clearSquiggles() {
	if (editor.AnyCursorHasSelection()) {
		for (size_t i = 0; i < editor.GetNumberOfCursors(); i++) {
			auto selection = editor.GetCursorSelection(i);

			if (selection.start != selection.end) {
				editor.ClearSquiggles(selection.start, selection.end);
			}
		}

	} else {
		editor.ClearSquiggles();
	}
}
