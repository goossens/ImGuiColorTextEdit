//	TextEditor - A syntax highlighting text editor for ImGui
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <algorithm>
#include <functional>
#include <string>

#include "../TextEditor.h"
#include "../TextDiff.h"
#include "../extras/LspBridge.h"
#include "../extras/Notifications.h"
#include "../extras/TrieAutoComplete.h"


//
//	Editor
//

class Editor {
public:
	// constructor
	Editor();

	// file related functions
	void newFile();
	void openFile();
	void openFile(const std::string& path);
	void saveFile();

	// manage program exit
	void tryToQuit();
	inline bool isDone() const { return done; }

	// render the editor
	void render();

	// debugging support
	inline void setDebugInformation(std::function<std::string()> callback) { getBackendDebugInformation = callback; }

private:
	// private functions
	void renderMenuBar();
	void renderStatusBar();

	void showDiff();
	void showFileOpen();
	void showSaveFileAs();
	void showConfirmClose(std::function<void()> callback);
	void showConfirmQuit();
	void showError(const std::string& message);
	void showAddSquiggle();
	void showClearSquiggles();

	void renderDiff();
	void renderFileOpen();
	void renderSaveAs();
	void renderConfirmClose();
	void renderConfirmQuit();
	void renderConfirmError();
	void renderAddSquiggle();
	void renderClearSquiggle();

	void renderDebugInformation();

	inline bool isDirty() const { return editor.GetUndoIndex() != version; }
	inline bool isSavable() const { return isDirty() && filename != "untitled"; }

	// properties
	std::string originalText;
	TextEditor editor;
	TextDiff diff;
	std::string filename;
	size_t version;
	bool done = false;
	bool popup = true;
	std::string errorMessage;
	std::function<void()> onConfirmClose;

	TextEditor::LineBreakConfig lineBreakConfig;

	float fontSize = 17.0f;
	inline void resetFontSize() { fontSize = 17.0f; }
	inline void increaseFontSize() { fontSize = std::clamp(fontSize + 1.0f, 8.0f, 24.0f); }
	inline void decreaseFontSize() { fontSize = std::clamp(fontSize - 1.0f, 8.0f, 24.0f); }

	inline void setDarkPalette() {
		ImGui::StyleColorsDark();
		editor.SetPalette(editor.GetDarkPalette());
		diff.SetPalette(editor.GetDarkPalette());
	}

	inline void setLightPalette() {
		ImGui::StyleColorsLight();
		editor.SetPalette(editor.GetLightPalette());
		diff.SetPalette(editor.GetLightPalette());
	}

	void setLanguage(const TextEditor::Language* language);
	void setLanguageByName(const std::string& name);
	void setLanguageByExtention(const std::string& filename);

	// examples
	void toggleNavigationMode();
	void toggleTrieAutoComplete();
	void toggleLspBridge();
	void toggleShowWordAtMouse();
	void toggleLineMarkers();
	void toggleLineDecorator();
	void toggleContextMenus();
	void toggleLineBreak();
	void clearSquiggles();

	bool demoTrieAutoComplete = false;
	bool demoLspBridge = false;
	bool showWordAtMouse = false;
	bool showLineMarkers = false;
	bool showLineDecorator = false;
	bool showContextMenus = false;
	bool enableUnicodeLineBreakAlgorithm = false;
	bool showDebugInformation = false;

	TrieAutoComplete trieAutoComplete;
	LspBridge lsp;
	static constexpr int lspOptions = LspBridge::autocomplete | LspBridge::showHoverHelp;

	std::function<std::string()> getBackendDebugInformation;

	size_t squiggleType = 1;
	ImColor squiggleColor{1.0f, 0.2f, 0.0f, 0.8f};
	char squiggleToolTip[128] = {};

	// notification system
	Notifications notifications;

	// editor state
	enum class State {
		edit,
		diff,
		newFile,
		openFile,
		saveFileAs,
		confirmClose,
		confirmQuit,
		confirmError,
		addSquiggle,
		clearSquiggles
	} state = State::edit;
};
