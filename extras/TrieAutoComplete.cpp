//	TextEditor - A syntax highlighting text editor for ImGui
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "TrieAutoComplete.h"


//
//	TrieAutoComplete::TrieAutoComplete
//

TrieAutoComplete::TrieAutoComplete() {
}


//
//	TrieAutoComplete::Connect
//

void TrieAutoComplete::Connect(TextEditor* newEditor) {
	// disconnect first (if required)
	if (editor) {
		Disconnect();
	}

	// remember current editor
	editor = newEditor;

	// rebuild word list
	buildTrie();

	// setup autocomplete by submitting a new configuration
	TextEditor::AutoCompleteConfig config;

	config.callback = [this](TextEditor::AutoCompleteState& state) {
		trie.findSuggestions(state.suggestions, state.searchTerm);
	};

	editor->SetAutoCompleteConfig(&config);

	// enable tracking of language changes
	editor->SetLanguageChangeCallback([this]() {
		buildTrie();
	});

	// enable change tracking
	// we don't track every keystroke, callbacks can be delayed up to 3000 milliseconds
	// if you want live tracking, change the 2000 to 0 (performance hit will be minimal for small documents)
	editor->SetChangeCallback([this]() {
		buildTrie();
	}, 2000);
}


//
//	TrieAutoComplete::Disconnect
//

void TrieAutoComplete::Disconnect() {
	// disconnect from text editor (if required)
	if (editor) {
		editor->SetAutoCompleteConfig(nullptr);
		editor->SetLanguageChangeCallback(nullptr);
		editor->SetChangeCallback(nullptr);
		editor = nullptr;
	}
}


//
//	TrieAutoComplete::buildTrie
//

void TrieAutoComplete::buildTrie() {
	// empty list first
	trie.clear();

	// add language words (if required)
	auto language = editor->GetLanguage();

	if (language) {
		for (auto& word : language->keywords) { trie.insert(word); }
		for (auto& word : language->declarations) { trie.insert(word); }
		for (auto& word : language->identifiers) { trie.insert(word); }
	}

	// add all identifiers in current document
	editor->IterateIdentifiers([this](const std::string& identifier) {
		trie.insert(identifier);
	});
}


//
//	TrieAutoComplete::Trie::insert
//

void TrieAutoComplete::Trie::insert(const std::string_view& word) {
	auto node = root.get();
	auto end = word.end();
	auto i = TextEditor::CodePoint::skipBOM(word.begin(), end);

	while (i < end) {
		ImWchar codepoint;
		i = TextEditor::CodePoint::read(i, end, &codepoint);

		if (node->children.find(codepoint) == node->children.end()) {
			node->children[codepoint] = std::make_unique<Node>();
		}

		node = node->children[codepoint].get();
	}

	node->word = word;
}


//
//	TrieAutoComplete::Trie::findSuggestions
//

void TrieAutoComplete::Trie::findSuggestions(std::vector<std::string>& suggestions, const std::string_view& searchTerm, size_t limit, size_t maxSkippedLetters) {
	// clear result vector
	maxSkip = maxSkippedLetters;
	suggestions.clear();

	// don't even try if search term is empty
	if (searchTerm.size() != 0) {
		// convert search term into vector of code blocks
		searchCodepoints.clear();
		auto end = searchTerm.end();
		auto i = TextEditor::CodePoint::skipBOM(searchTerm.begin(), end);

		while (i < end) {
			ImWchar codepoint;
			i = TextEditor::CodePoint::read(i, end, &codepoint);
			searchCodepoints.emplace_back(codepoint);
		}

		// recursively evaluate nodes
		candidates.clear();
		evaluateNode(root.get(), 0, 0, maxSkip);

		// did we find anything?
		if (candidates.size()) {
			// sort candidates by cost
			std::sort(candidates.begin(), candidates.end());

			// remove duplicates which are caused by mutiple paths based on skips
			auto last = std::unique(candidates.begin(), candidates.end());
			candidates.erase(last, candidates.end());

			// populate suggestions (applying limit)
			auto size = std::min(static_cast<size_t>(limit), candidates.size());

			for (size_t j = 0; j < size; j++) {
				suggestions.emplace_back(candidates[j].node->word);
			}
		}
	}
}


//
//	TrieAutoComplete::Trie::evaluateNode
//

void TrieAutoComplete::Trie::evaluateNode(const Node* node, size_t index, size_t cost, size_t skip) {
	// see if that is one of our children (check both lower and uppercase matches)
	ImWchar codepointLower = TextEditor::CodePoint::toLower(searchCodepoints[index]);
	Node* childLower = nullptr;

	if (node->children.find(codepointLower) != node->children.end()) {
		// codepoint found, is this the last one in our searchTerm?
		childLower = node->children.at(codepointLower).get();

		if (index == searchCodepoints.size() - 1) {
			// yes, add candidate words to results
			addCandidates(childLower, cost);

		} else {
			// no, try to find the rest
			evaluateNode(childLower, index + 1, cost, maxSkip);
		}
	}

	ImWchar codepointUpper = TextEditor::CodePoint::toUpper(searchCodepoints[index]);
	Node* childUpper = nullptr;

	if (node->children.find(codepointUpper) != node->children.end()) {
		// codepoint found, is this the last one in our searchTerm?
		childUpper = node->children.at(codepointUpper).get();

		if (index == searchCodepoints.size() - 1) {
			// yes, add candidate words to results
			addCandidates(childUpper, cost);

		} else {
			// no, try to find the rest
			evaluateNode(childUpper, index + 1, cost, maxSkip);
		}
	}

	// also try children to support detection of missing letters (if we haven't skipped too many entries yet)
	if (skip) {
		for (auto const& [key, value] : node->children) {
			auto next = value.get();

			if (next != childLower && next != childUpper) {
				evaluateNode(next, index, cost + 1, skip - 1);
			}
		}
	}
}


//
//	TrieAutoComplete::Trie::addCandidates
//

void TrieAutoComplete::Trie::addCandidates(const Node* node, size_t cost) {
	if (node->word.size()) {
		candidates.emplace_back(node, cost);
	}

	for (auto const& [key, value] : node->children) {
		addCandidates(value.get(), cost + 1);
	}
}
