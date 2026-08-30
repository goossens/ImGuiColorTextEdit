# Custom Line Numbers

By default, the text editor renders line numbers right aligned for each line when configured. To allow other alignments,
interval-based numbering, custom colors or animations, the text editor offers a custom rendering callback that enables the application to render line numbers anyway it sees fit. By using the SetCustomLineNumberRenderer,
ClearCustomLineNumberRenderer and HasCustomLineNumberRenderer API calls, an application has full control over the rendering.

A custom renderer (which will be called for each line number to be rendered) receives a reference to a TextEditor::CustomLineNumber
structure that contains the following information:

```c++
	struct CustomLineNumber {
		// draw list to submit rendering commands to
		ImDrawList* drawList;

		// top left corner of line number box
		// can be used directly to submit drawing commands
		ImVec2 pos;

		// visible size of line number box in pixels
		ImVec2 size;

		// width of line number box in glyphs (this is variable)
		// the editor calculates the number of digits required for the highest line number
		size_t digits;

		// line number to be rendered (zero-based)
		size_t lineNumber;

		// line number for current cursor (zero-based)
		size_t cursorLineNumber;

		// line number color from current palette
		// this can be ignored if custom renderer has its own palette or animation
		ImU32 color;
	};
```

As an example, the code below renders the line number left justified with an interval of 10 and renders a hyphen on the 5s
and a dot on the other lines. It also uses the current palette's color to highlight the current line. You can see this in
action in the example application.

```c++
	editor.SetCustomLineNumberRenderer([](const TextEditor::CustomLineNumber& data) {
		std::string buffer;
		auto lineNo = data.lineNumber + 1;

		if ((data.lineNumber == data.cursorLineNumber) || ((lineNo % 10) == 0)) {
			buffer = std::to_string(data.lineNumber + 1);

		} else if ((lineNo % 5) == 0) {
			buffer = "-";

		} else {
			buffer = " .";
		}

		data.drawList->AddText(data.pos, data.color, buffer.c_str());
	});
```