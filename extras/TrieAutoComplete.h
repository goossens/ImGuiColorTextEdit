//	TextEditor - A syntax highlighting text editor for Dear ImGui.
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../TextEditor.h"


//
//	TrieAutoComplete
//
//	This is a utility class that can be connected to an TextEditor instance
//	and it will provide simple autocomplete features based on the language
//	set in the editor and identifiers contained in the text.
//
//	This feature is not context aware and for that, you should connect
//	to a language server using the LspBridge class also contained in this
//	directory.
//
//	TrieAutoComplete is standalone, simple, fast but not context accurate.
//	LspBridge has dependencies and is more complex.
//

class TrieAutoComplete {
public:
	// constructor
	TrieAutoComplete();

	// manage connection to text editor
	void Connect(TextEditor* editor);
	void Disconnect();
	inline bool IsConnected() const { return editor != nullptr; }

private:
	// the text editor we are connected to
	TextEditor* editor = nullptr;

	// support functions
	void buildTrie();

	// the trie structure containing possible solutions
	class Trie {
	public:
		// constructor
		Trie() { clear(); }

		// clear word tree
		inline void clear() { root = std::make_unique<Node>(); }

		// insert word (UTF-8 encoded) into tree
		void insert(const std::string_view& word);

		// populate list of suggestions based on provided search term (which is UTF-8 encoded)
		// limit is maximum number of suggestions returned after they are sorted by relevance
		// maxSkippedLetters is a the largest number of letters that can be skipped to find the next match
		// this allows for missing letters (out of order letters are not taken into account)
		void findSuggestions(std::vector<std::string>& suggestions, const std::string_view& searchTerm, size_t limit=20, size_t maxSkippedLetters=2);

	private:
		// definition of single node in the word graph
		struct Node {
			std::unordered_map<ImWchar, std::unique_ptr<Node>> children;
			std::string word;
		};

		// the root node
		std::unique_ptr<Node> root;

		// maximum number of letters that can be skipped skip in matching algorithm
		size_t maxSkip;

		// search term as codepoint vector
		std::vector<ImWchar> searchCodepoints;

		// possible autocomplete candidates
		struct Candidate {
			Candidate(const Node* n, size_t c) : node(n), cost(c) {}
			bool operator<(const Candidate& rhs) const { return cost < rhs.cost; }
			bool operator==(const Candidate& rhs) const { return node == rhs.node; }
			const Node* node;
			size_t cost;
		};

		std::vector<Candidate> candidates;

		// utility functions
		void evaluateNode(const Node* node, size_t index, size_t cost, size_t skip);
		void addCandidates(const Node* node, size_t cost);
	} trie;
};
