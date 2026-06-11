//	TextEditor - A syntax highlighting text editor for ImGui
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <iostream>
#include <sstream>
#include <utility>

#include "lsp/io/stream.h"

#include "LspBridge.h"


//
//	LspBridge::LspBridge
//

LspBridge::LspBridge() {
}


//
//	LspBridge::~LspBridge
//

LspBridge::~LspBridge() {
	if (IsRunning()) {
		Stop();
	}
}


//
//	LspBridge::Start
//

bool LspBridge::Start(const std::string& rootPath, const std::string& executable, const std::vector<std::string>& args) {
	try {
		// start a new language server
		process = lsp::Process(executable, args);
		connection = std::make_unique<lsp::Connection>(process.stdIO());
		messageHandler = std::make_unique<lsp::MessageHandler>(*connection);
		running.store(true);

		// start a thread to handle message exchanges
		messageThread = std::thread([this](){
			messageLoop();
		});

		// setup initialize params with client capabilities
		auto initializeParams = lsp::requests::Initialize::Params();
		initializeParams.processId = lsp::Process::currentProcessId();
		initializeParams.rootUri = lsp::DocumentUri::fromPath(rootPath);

		initializeParams.capabilities = {
			.textDocument = lsp::TextDocumentClientCapabilities {
				// .completion = lsp::CompletionClientCapabilities {
				// 	.completionItem = lsp::CompletionClientCapabilitiesCompletionItem {
				// 		.insertTextModeSupport = lsp::CompletionClientCapabilitiesCompletionItemInsertTextModeSupport {
				// 			.valueSet = lsp::Array<lsp::InsertTextModeEnum>{
				// 				lsp::InsertTextMode::AsIs
				// 			}
				// 		}
				// 	}
				// },
				.hover = lsp::HoverClientCapabilities {
					.contentFormat = {
						{lsp::MarkupKind::PlainText}
					}
				}
			},
			.general = lsp::GeneralClientCapabilities {
				.positionEncodings = lsp::Array<lsp::PositionEncodingKindEnum>{
					lsp::PositionEncodingKind::UTF32
				}
			}
		};

		// send initialize request to the server and wait for the response
		auto initializeRequest = messageHandler->sendRequest<lsp::requests::Initialize>(std::move(initializeParams));
		auto initializeResult = initializeRequest.result.get();

		serverCapabilities = std::move(initializeResult.capabilities);

		// send the 'initialized' notification to let the server know that the client is ready
		messageHandler->sendNotification<lsp::notifications::Initialized>({});
		return true;

	} catch(const std::exception& e) {
		std::stringstream ss;
		ss << "Can't start language server[" << executable << "], error: " << e.what();
		errorMessage = ss.str();
		return false;
	}
}


//
//	LspBridge::Stop
//

void LspBridge::Stop() {
	if (IsRunning()) {
		// clear all open documents (this also closes them)
		documents.clear();

		// send a shutdown request and wait for response
		messageHandler->sendRequest<lsp::requests::Shutdown>(
			[this](const lsp::requests::Shutdown::Result&) {
				// send an exit notification
				messageHandler->sendNotification<lsp::notifications::Exit>();
				running.store(false);
			},
			[](const lsp::ResponseError& error) {
				std::cerr << "Error: " << error.code() << " - " << error.what() << std::endl;
			});

		// wait for message handling thread to finish
		if (messageThread.joinable()) {
			messageThread.join();
		}
	}
}


//
//	LspBridge::messageLoop
//

void LspBridge::messageLoop() {
	while (IsRunning()) {
		try {
			messageHandler->processIncomingMessages();

		} catch(const std::exception& e) {
			std::cerr << "================================ Error: " << e.what() << std::endl;
		}
	}
}


//
//	LspBridge::o
//

void LspBridge::OpenDocument(const std::string& path, TextEditor& editor, int options) {
	if (IsRunning()) {
		if (documents.find(path) != documents.end()) {
			documents.erase(path);
		}

		documents.try_emplace(path, *this, path, editor, options);
	}
}


//
//	LspBridge::CloseDocument
//

void LspBridge::CloseDocument(const std::string& path) {
	if (IsRunning()) {
		if (documents.find(path) != documents.end()) {
			documents.erase(path);
		}
	}
}


//
//	LspBridge::IsOpen
//

bool LspBridge::IsOpen(const std::string& path) {
	return documents.find(path) != documents.end();
}


//
//	LspBridge::update
//

void LspBridge::Update(const std::string& path) {
	auto i = documents.find(path);

	if (i != documents.end()) {
		i->second.update();
	}
}


//
//	LspBridge::Document::Document
//

LspBridge::Document::Document(LspBridge& bridge, const std::string& path, TextEditor& editor, int options)
	: bridge(bridge), path(path), editor(editor), options(options) {

	// sanitize options (can't hover and signature at the same time)
	if ((options & showHoverHelp) && (options & showSignature)) {
		options &= ~showSignature;
	}

	// open document in language server
	bridge.messageHandler->sendNotification<lsp::notifications::TextDocument_DidOpen>({
		.textDocument = {
			.uri = lsp::DocumentUri::fromPath(path),
			.languageId = editor.GetLanguageName(),
			.version = version++,
			.text = editor.GetText()
		}
	});

	// provide document text to language server
	lsp::Array<lsp::TextDocumentContentChangeEvent> contentChanges;

	contentChanges.emplace_back(lsp::TextDocumentContentChangeEvent_Text({
		.text = editor.GetText()
	}));

	lsp::DidChangeTextDocumentParams params;
	params.textDocument.uri = lsp::DocumentUri::fromPath(path);
	params.textDocument.version = version++;
	params.contentChanges = std::move(contentChanges);
	bridge.messageHandler->sendNotification<lsp::notifications::TextDocument_DidChange>(std::move(params));

	// track changes in document from now on and pass them to the language server
	editor.SetTransactionCallback([&](const std::vector<TextEditor::Change>& changes) {
		lsp::Array<lsp::TextDocumentContentChangeEvent> contentChanges;

		for (auto& change : changes) {
			if (change.insert) {
				contentChanges.emplace_back(lsp::TextDocumentContentChangeEvent_Range_Text({
					.range = lsp::Range({
						.start = lsp::Position({
							.line = static_cast<lsp::uint>(change.start.line),
							.character = static_cast<lsp::uint>(change.start.index)
						}),
						.end = lsp::Position({
							.line = static_cast<lsp::uint>(change.start.line),
							.character = static_cast<lsp::uint>(change.start.index)
						})
					}),
					.text = change.text
				}));

			} else {
				contentChanges.emplace_back(lsp::TextDocumentContentChangeEvent_Range_Text({
					.range = lsp::Range({
						.start = lsp::Position({
							.line = static_cast<lsp::uint>(change.start.line),
							.character = static_cast<lsp::uint>(change.start.index)
						}),
						.end = lsp::Position({
							.line = static_cast<lsp::uint>(change.end.line),
							.character = static_cast<lsp::uint>(change.end.index)
						})
					}),
					.text = ""
				}));
			}
		}

		lsp::DidChangeTextDocumentParams params;
		params.textDocument.uri = lsp::DocumentUri::fromPath(path);
		params.textDocument.version = version++;
		params.contentChanges = std::move(contentChanges);
		bridge.messageHandler->sendNotification<lsp::notifications::TextDocument_DidChange>(std::move(params));
	});

	if (options & autocomplete) {
		TextEditor::AutoCompleteConfig config;

		config.callback = [&](TextEditor::AutoCompleteState& state) {
			bridge.messageHandler->sendRequest<lsp::requests::TextDocument_Completion>(
				{
					{
						.textDocument = {
							.uri = lsp::DocumentUri::fromPath(path)
						},
						.position = lsp::Position {
							.line = static_cast<lsp::uint>(state.searchTermEnd.line),
							.character = static_cast<lsp::uint>(state.searchTermEnd.index)
						}
					}
				},
				[&](const lsp::requests::TextDocument_Completion::Result& result) {
					// if (result.holdsAlternative<lsp::CompletionList>()) {
					if (!result.isNull()) {
						std::scoped_lock lock(mutex);
						suggestions.clear();
						auto list = result.get<lsp::CompletionList>();

						for (auto& item : list.items) {
							if (item.insertText) {
								suggestions.emplace_back(*item.insertText);
							}
						}
					}
				},
				[](const lsp::ResponseError& error) {
					std::cerr << "Error: " << error.code() << " - " << error.what() << std::endl;
				});

			state.suggestionsPromise = true;
		};

		editor.SetAutoCompleteConfig(&config);
	}
}


//
//	LspBridge::Document::~Document
//

LspBridge::Document::~Document() {
	// close document in language server
	bridge.messageHandler->sendNotification<lsp::notifications::TextDocument_DidClose>({
		.textDocument = {
			.uri = lsp::DocumentUri::fromPath(path)
		}
	});

	// disconnect from editor
	editor.SetTransactionCallback(nullptr);

	if (options & autocomplete) {
		editor.SetAutoCompleteConfig(nullptr);
	}

	if ((options & showHoverHelp) || (options & showSignature)) {
		if (editor.HasTextHoverCallback()) {
			editor.SetTextHoverCallback(nullptr);
		}
	}
}


//
//	LspBridge::Document::update
//

void LspBridge::Document::update() {
	// see if we are hovering over text
	if (editor.IsMousePosOverGlyph(ImGui::GetMousePos())) {
		auto docPos = editor.GetDocPosAtMousePos(ImGui::GetMousePos());

		if (docPos != mouseDocPos) {
			mouseDocPos = docPos;

			if (options & showSignature) {
				bridge.messageHandler->sendRequest<lsp::requests::TextDocument_SignatureHelp>(
					{
						{
							.textDocument = {
								.uri = lsp::DocumentUri::fromPath(path)
							},
							.position = lsp::Position {
								.line = static_cast<lsp::uint>(docPos.line),
								.character = static_cast<lsp::uint>(docPos.index)
							}
						}
					},
					[&](const lsp::requests::TextDocument_SignatureHelp::Result& result) {
						if (!result.isNull()) {
							signatures.clear();

							for (auto& signature : result->signatures) {
								signatures.emplace_back(signature.label);
							}
						}
					},
					[](const lsp::ResponseError& error) {
						std::cerr << "Error: " << error.code() << " - " << error.what() << std::endl;
					});

			} else if (options & showHoverHelp) {
				bridge.messageHandler->sendRequest<lsp::requests::TextDocument_Hover>(
					{
						{
							.textDocument = {
								.uri = lsp::DocumentUri::fromPath(path)
							},
							.position = lsp::Position {
								.line = static_cast<lsp::uint>(docPos.line),
								.character = static_cast<lsp::uint>(docPos.index)
							}
						}
					},
					[this](const lsp::requests::TextDocument_Hover::Result& result) {
						if (!result.isNull()) {
							std::scoped_lock lock(mutex);

							if (std::holds_alternative<lsp::MarkupContent>(result->contents)) {
								auto message = std::get<lsp::MarkupContent>(result->contents);
								hoverMessage = message.value;

							} else {
								hoverMessage.clear();
							}

						} else {
							hoverMessage.clear();
						}
					},
					[](const lsp::ResponseError& error) {
						std::cerr << "Error: " << error.code() << " - " << error.what() << std::endl;
					});
			}
		}
	}

	// submit suggestions asynchronously (if required)
	if ((options & autocomplete) && suggestions.size()) {
		std::scoped_lock lock(mutex);
		editor.SetAutoCompleteSuggestions(suggestions);
		suggestions.clear();
	}

	// setup hover help (if required)
	if (options & showHoverHelp) {
		std::scoped_lock lock(mutex);

		if (hoverMessage.size() && editor.IsMousePosOverGlyph(ImGui::GetMousePos())) {
			editor.SetTextHoverCallback([&](TextEditor::PopupData) {
				ImGui::TextDisabled("%s", hoverMessage.c_str());
			});

		} else {
			editor.ClearTextHoverCallback();
		}
	}

	// setup signatures (if required)
	if (options & showSignature) {
		std::scoped_lock lock(mutex);

		if (signatures.size() && editor.IsMousePosOverGlyph(ImGui::GetMousePos())) {
			editor.SetTextHoverCallback([&](TextEditor::PopupData) {
				for (auto& signature : signatures) {
					ImGui::TextDisabled("%s", signature.c_str());
				}
			});

		} else {
			editor.ClearTextHoverCallback();
		}
	}
}
