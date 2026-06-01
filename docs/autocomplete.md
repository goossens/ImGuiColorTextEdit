# AutoComplete

The text editor provides an optional autocomplete framework. Once configured, the editor
takes care of activation events (triggering), state tracking, visualization and insertion
of suggestions with full undo/redo. The application is responsible for providing the list
of suggestions through a callback or the API. This allows simple implementations to provide
suggestions in realtime and allows other implementations to do things asynchronously like
reaching out to a language server. AutoComplete can't be triggered when multiple cursors
are active as this causes a mess. Try it in Visual Studio Code if you want to see what I mean.

![AutoComplete](autocomplete.png)

To activate the feature, the app must provide a configuration like:

```c++
TextEditor::AutoCompleteConfig config;

config.callback = [this](TextEditor::AutoCompleteState& state) {
	....
};

editor.SetAutoCompleteConfig(&config);
```

Deactivation can be achieved by passing **nullptr** to **SetAutoCompleteConfig**.
The **TextEditor::AutoCompleteConfig** class contains all the configuration options
and is defined as (please note defaults):

```c++
class AutoCompleteConfig {
public:
	// specifies whether typing by the user triggers autocomplete
	bool triggerOnTyping = true;

	// specifies whether the specified shortcut triggers autocomplete
	bool triggerOnShortcut = true;

	// specifies whether typing (or shortcut) in comments or strings triggers autocomplete
	bool triggerInComments = false;
	bool triggerInStrings = false;

	// manual trigger key sequence (default is Ctrl+space on all platforms, even MacOS)
	// remember Dear ImGui reverses Ctrl and Command on MacOS
#if __APPLE__
	ImGuiKeyChord triggerShortcut = ImGuiMod_Super | ImGuiKey_Space;
#else
	ImGuiKeyChord triggerShortcut = ImGuiMod_Ctrl | ImGuiKey_Space;
#endif

	// see if single suggestions are automatically inserted
	// this only works when triggered manually
	bool autoInsertSingleSuggestions = false;

	// delay in milliseconds between autocomplete trigger and suggestions popup
	std::chrono::milliseconds triggerDelay(200);

	// text label used when no suggestions are available (this allows for internationalization)
	std::string noSuggestionsLabel = "No suggestions";

	// width of suggestion popup expressed in number of glyphs
	size_t suggestionWidth = 30;

	// called when autocomplete is configured, active and the editor needs an updated suggestions list
	// callback must populate and order suggestions in state object
	// suggestion list is not cleared by editor between callbacks
	// callback is called during the rendering process (so don't take too long)

	// if it takes too long, applications should do search in separate thread and
	// use API to report results (see SetAutoCompleteSuggestions)
	std::function<void(AutoCompleteState&)> callback;

	// optional opaque void* that must be managed externally but passed to callback
	void* userData = nullptr;
};
```

When the callback is activated, a **TextEditor::AutoCompleteState** object is passed
informing the app about the context and providing space to return suggestions.
It is defined as:

```c++
class AutoCompleteState {
public:
	// current context
	std::string searchTerm;
	DocPos searchTermStart;
	DocPos searchTermEnd;

	bool inIdentifier;
	bool inNumber;
	bool inComment;
	bool inString;

	// currently selected language (could be nullptr if no language is selected)
	const Language* language;

	// optional opaque void* provided by app when autocomplete was setup
	void* userData;

	// auto complete suggestions te be provided by app callback (the app is responsible for sorting)

	// the editor does not automatically include language specific keywords or identifiers in the suggestion list
	// this is left to the application so it can be context specific in case a language server is used
	// a pointer to the current language definition is provided so callbacks have easy access
	std::vector<std::string> suggestions;

	// set this to true if you are building the suggestion list asynchronously and provide it later
	// this way autocomplete is not cancelled if the suggestion list is empty and the user hits tab or enter
	bool suggestionsPromise = false;
};
```

The editor comes with two autocomplete solutions that are in the extras
folder. The first is a simple suggestion generator that attaches itself to
the text editor and create a suggestion list based on keywords in the selected
language and all identifiers contained in the current document. This
solution is very fast but lacks the ability to provide context specific
suggestions. For that, a second solution is [available](lsp.md) which lets
you connect to a language server.

To use the simple solution, do the following:

- Include the **TrieAutoComplete.cpp** and **TrieAutoComplete.h** files in your
project (both are in the **extras** folder).
- Create an instance of the **TrieAutoComplete** class.
- Use the **Connect** and **Disconnect** methods to connect the object to the text editor.
- Everything else is done automatically for you.

So as a simple example:

```c++
TextEditor editor;
TrieAutoComplete trieAutoComplete;

trieAutoComplete.Connect(&editor);
```

For more information on the Language Server Protocol Bridge, its dependencies
and its use, see the [documentation](lsp.md).
