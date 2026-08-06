# Custom Cursor Caret

By default, the text editor renders a vertical line as the cursor caret which can be hidden using text editor options or made to blink based on Dear ImGui's configuration. The blink algorithm is the same as Dear ImGui to achieve a consistent look-and-feel.

For those that want an underline or block caret, the text editor offers a custom rendering callback that enables the application to render whatever they want (even a pulsating pink Barbie emoji). By using the SetCustomCaretRenderer, ClearCustomCaretRenderer and HasCustomCaretRenderer API calls, an application has full control over rendering of the caret.

A custom renderer (which might be called multiple times during a frame when multiple cursors are active) receives a reference to a TextEditor::CustomCaret structure that contains the following information:

```c++
	struct CustomCaret {
		// draw list to submit rendering commands to
		ImDrawList* drawList;

		// top left corner of glyph where cursor is (in screen coordinates)
		// can be used directly to submit drawing commands
		ImVec2 glyphPos;

		// visible size of glyph
		ImVec2 glyphSize;

		// flag indicating if cursor is visible (based on configuration and standard blinking algorithm)
		// this can be ignored if the custom caret has its own animation algorithm
		bool caretVisible;

		// color of cursor caret as per the current palette
		// that can also be ignored if custom caret has its own palette of animation
		ImU32 caretColor;

		// index of the cursor being rendered (in case additional cursor information is required)
		size_t cursorIndex;
	};
```

As an example, rendering a custom block caret that has 50% opacity, the application can do something as simple as:

```c++
	editor.SetCustomCaretRenderer([](const TextEditor::CustomCaret& caret) {
		if (caret.caretVisible) {
			auto color = ImGui::GetColorU32(caret.caretColor, 0.5f);
			caret.drawList->AddRectFilled(caret.glyphPos, caret.glyphPos + caret.glyphSize, color);
		}
	});
```