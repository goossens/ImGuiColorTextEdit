//	TextEditor - A syntax highlighting text editor for ImGui
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "lsp/messagehandler.h"
#include "lsp/types.h"
#include "lsp/process.h"

#include "../TextEditor.h"


//
//	LspBridge
//

class LspBridge {
public:
	// constructor/destructor
	LspBridge();
	~LspBridge();

	// start/stop the server
	bool Start(const std::string& rootPath, const std::string& executable, const std::vector<std::string>& args={});
	void Stop();

	// features for a specific document
	static constexpr int autocomplete = 1 << 0;
	static constexpr int showSignature = 1 << 1;
	static constexpr int showHoverHelp = 1 << 2;

	// open/close documents (server can handle multiple at the time)
	void OpenDocument(const std::string& path, TextEditor& editor, int options=autocomplete);
	void CloseDocument(const std::string& path);
	bool IsOpen(const std::string& path);

	// call this before TextEditor is rendered
	void Update(const std::string& path);

	// get status
	inline bool IsRunning() const { return running.load(); }
	inline const std::string& GetError() const { return errorMessage; }

private:
	// work variables
	friend class Document;
	lsp::Process process;
	std::string errorMessage;

	std::unique_ptr<lsp::Connection> connection;
	std::unique_ptr<lsp::MessageHandler> messageHandler;
	lsp::ServerCapabilities serverCapabilities;

	std::atomic_bool running = false;
	std::thread messageThread;

	// a single open document
	class Document {
	public:
		// constructor/destructor
		Document(LspBridge& bridge, const std::string& path, TextEditor& editor, int options);
		~Document();

		// update document state
		void update();

	private:
		// properties
		LspBridge& bridge;
		const std::string path;
		TextEditor& editor;
		int options;
		int version = 1;

		std::mutex mutex;
		std::vector<std::string> suggestions;
		std::vector<std::string> signatures;
		std::string hoverMessage;

		TextEditor::DocPos mouseDocPos;
	};

	// all documents currently open
	std::unordered_map<std::string, Document> documents;

	// support functions
	void messageLoop();
};
