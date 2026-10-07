# Squiggles

The text editor provides an option to mark ranges of text with squiggles. A squiggle is drawn as a wavy underline (the default, like errors and warnings in Visual Studio Code) or as a background behind the text (for instance to highlight search results). Squiggles can have individual colors and can optionally have tooltips.

Each squiggle has a type, a number chosen by the application, so that all the squiggles of one category can be cleared at once (for instance the results of a search, while the errors reported by a compiler stay).

Squiggles are attached to glyphs, so when text is inserted or deleted before them, they move as well. When a glyph with a squiggle is deleted, the squiggle disappears from that glyph. A glyph has at most one squiggle: a new squiggle replaces the ones under it.

Squiggles are rendered after selected text is highlighted, so if you want to see both, please use a transparent color for background squiggles.

```c++
static constexpr size_t errors = 1;
static constexpr size_t searchResults = 2;

// a wavy underline with a tooltip
editor.AddSquiggle(TextEditor::DocPos(line, start), TextEditor::DocPos(line, end), errors, IM_COL32(255, 0, 0, 255), errorMessage);

// backgrounds, replaced when the search changes
editor.ClearSquiggles(searchResults);

for (auto& result : results) {
	editor.AddSquiggle(result.start, result.end, searchResults, IM_COL32(255, 200, 0, 100), "", TextEditor::SquiggleStyle::background);
}
```

To show something next to a squiggle (a popup, a list of search results that scrolls to them), **DocPos2ScreenPos** gives the screen position of a glyph after the editor was rendered. An editor out of view is not laid out: its positions are those of the last frame it was visible.

An application that provides its own search can disable the editor's find and replace window with **SetFindReplaceEnabled(false)**. The editor then leaves Ctrl-F, Shift-Ctrl-F and Ctrl-G to the application.
