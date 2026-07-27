# Context Menus

The text editor provides an option to create context menus when the user right clicks on a line number or anywhere in the text. The editor creates these menus and a provided callback needs to populate them.

In the example below, the line number context menu is used to control breakpoints and the text context menu is used to report the current position in the text. Only the first is depicted in the screenshot.

The callback is provided with a PopupData struct to have quick access to the location in the document
as well as any potential [user data](userdata.md). The text Editor only shows the popup when the mouse is over a glyph.

```c++
struct PopupData {
	DocPos pos;
	void* userData;
};
```

The following is taken from the [example application](../example) so you can check there to see it in action.


```c++
editor.SetLineNumberContextMenuCallback([]([[maybe_unused]] TextEditor::PopupData& data) {
	if (ImGui::MenuItem("Set Breakpoint")) { /* handle click */ }
	if (ImGui::MenuItem("Remove Breakpoint")) { /* handle click */ }
});

editor.SetTextContextMenuCallback([](const TextEditor::PopupData& data) {
	ImGui::Text("Line %zu, index %zu", data.pos.line + 1, data.pos.index + 1);
});
```

![Context Menu](contextMenus.png)
