#include "TextEditor.h"
#include "TextDiff.h"
#include <iostream>
#include <stdexcept>

static void check(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

static void frame(TextEditor& editor) {
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(800, 600));
    ImGui::Begin("regression");
    editor.Render("editor", ImVec2(700, 450));
    ImGui::End();
    ImGui::Render();
}

#ifdef __BALAZS__
static float whitespaceSpan(ImU32 color) {
    float left = std::numeric_limits<float>::max();
    float right = std::numeric_limits<float>::lowest();
    const auto* data = ImGui::GetDrawData();
    for (int i = 0; i < data->CmdListsCount; ++i) {
        for (const auto& vertex : data->CmdLists[i]->VtxBuffer) {
            if (vertex.col == color) {
                left = std::min(left, vertex.pos.x);
                right = std::max(right, vertex.pos.x);
            }
        }
    }
    check(right >= left, "tab arrow rendered");
    return right - left;
}
#endif

#ifdef __GEOGRAM__
struct Actions {
    int changes = 0;
    int saves = 0;
    int runs = 0;
    int stops = 0;
    int completions = 0;
};
static void callback(TextEditorAction action, void* data) {
    auto& actions = *static_cast<Actions*>(data);
    if (action == TEXT_EDITOR_TEXT_CHANGED) { ++actions.changes; }
    if (action == TEXT_EDITOR_SAVE) { ++actions.saves; }
    if (action == TEXT_EDITOR_RUN) { ++actions.runs; }
    if (action == TEXT_EDITOR_STOP) { ++actions.stops; }
    if (action == TEXT_EDITOR_COMPLETION) { ++actions.completions; }
}
#endif

int main() {
    try {
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.DisplaySize = ImVec2(1024, 768);
        io.DeltaTime = 1.0f / 60.0f;
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        TextEditor editor;
        editor.SetInsertSpacesOnTabs(false);
        editor.SetText("one\n\tUnicode: \xD0\x90");
        check(editor.GetLineCount() == 2, "UTF-8 line count");
        check(editor.GetText() == "one\n\tUnicode: \xD0\x90\n", "UTF-8 roundtrip");
        editor.SetText(std::u16string_view(u"wide\ntext"));
        check(editor.GetTextAsU16String() == u"wide\ntext\n", "UTF-16 roundtrip");
        check(TextEditor::LookupLanguage("Lua") != nullptr, "installed language registry");
        editor.SetLanguage(TextEditor::LookupLanguage("Lua"));
        frame(editor);

#ifdef __BALAZS__
        // Compare rendered geometry: the switch must affect the arrow, not
        // just store an unused flag, and must never change the text layout.
        editor.SetText("\tvalue");
        editor.SetShowTabsEnabled(true);
        check(!editor.IsShowingShortTabGlyphs(), "historical full-tab default");
        auto colors = TextEditor::GetDarkPalette();
        const ImU32 whitespaceColor = IM_COL32(1, 2, 3, 255);
        colors[static_cast<size_t>(TextEditor::Color::whitespace)] = whitespaceColor;
        editor.SetPalette(colors);
        frame(editor);
        const auto longSpan = whitespaceSpan(whitespaceColor);
        const auto textColumn = editor.DocPos2ScreenPos(TextEditor::DocPos(0, 1)).x;
        editor.SetShowShortTabGlyphs(true);
        frame(editor);
        check(editor.IsShowingShortTabGlyphs(), "short-tab mode");
        check(whitespaceSpan(whitespaceColor) < longSpan, "short arrow has smaller rendered span");
        check(editor.DocPos2ScreenPos(TextEditor::DocPos(0, 1)).x == textColumn, "tab layout unaffected");
        check(editor.GetLineText(0) == "\tvalue", "tab text unaffected");
#endif

#ifdef __GEOGRAM__
        Actions actions;
        editor.set_callback(callback, &actions);
        editor.SetText("hello");
        check(actions.changes == 1, "SetText callback");
        editor.SetText(std::u16string_view(u"hello"));
        check(actions.changes == 2, "Unicode SetText callback");
        editor.SetCursorPosition(TextEditor::CursorPosition(0, 5));
        frame(editor);
        editor.InsertText(" world");
        check(editor.GetText() == "hello world\n", "Geogram insert");
        check(actions.changes == 3, "insert callback exactly once");
        editor.Undo();
        check(editor.GetText() == "hello\n" && actions.changes == 4, "undo callback");
        editor.Redo();
        check(editor.GetText() == "hello world\n" && actions.changes == 5, "redo callback");
        editor.SetReadOnlyEnabled(true);
        editor.InsertText("ignored");
        check(editor.GetText() == "hello world\n" && actions.changes == 5, "read-only insert");
        editor.SetReadOnlyEnabled(false);
        editor.SelectAll();
        check(editor.HasSelection() && editor.GetSelection() == "hello world", "legacy selection helpers");
        check(editor.GetSelectionStart().column == 0, "legacy selection start");
        check(editor.GetLine(-1).empty(), "negative line is safe");
        editor.SetGeogramFindFont(io.Fonts->Fonts[0]);
        editor.SetGeogramCloseButtonLabel("close");
        editor.ShowFindAndReplaceDialog();
        frame(editor);
        editor.CloseFindReplaceWindow();
        editor.SetText("object.member = value");
        editor.SetFocus();
        frame(editor);
        auto point = editor.DocPos2ScreenPos(TextEditor::DocPos(0, 3));
        point.x += 1.0f;
        point.y += 1.0f;
        check(editor.GetWordContextAtScreenPos(point) == "object.member", "member completion context");
        auto separator = editor.DocPos2ScreenPos(TextEditor::DocPos(0, 13));
        separator.x += 1.0f;
        separator.y += 1.0f;
        check(editor.GetWordContextAtScreenPos(separator).empty(), "context stops at whitespace");
        check(editor.GetWordContextAtScreenPos(ImVec2(-100, -100)).empty(), "outside document context");
        io.AddKeyEvent(ImGuiKey_F2, true);
        frame(editor);
        io.AddKeyEvent(ImGuiKey_F2, false);
        frame(editor);
        check(actions.saves == 1, "F2 save action");
        io.AddKeyEvent(ImGuiKey_F5, true);
        frame(editor);
        io.AddKeyEvent(ImGuiKey_F5, false);
        frame(editor);
        check(actions.runs == 1, "F5 run action");
        editor.SetText("\tmember");
        editor.SetCursorPosition(TextEditor::CursorPosition(0, static_cast<int>(editor.GetTabSize())));
        frame(editor);
        check(editor.GetMainCursorPosition().index == 1, "legacy cursor columns account for tabs");
        editor.InsertText("!");
        check(editor.GetLine(0) == "\t!member", "insert after tab");
        const auto changesBeforeReadOnlyTab = actions.changes;
        editor.SetReadOnlyEnabled(true);
        io.AddKeyEvent(ImGuiKey_Tab, true);
        frame(editor);
        io.AddKeyEvent(ImGuiKey_Tab, false);
        frame(editor);
        check(actions.completions == 0 && actions.changes == changesBeforeReadOnlyTab, "read-only Tab does not request completion");
        editor.SetReadOnlyEnabled(false);
        const auto changesBeforeUnregister = actions.changes;
        editor.set_callback(nullptr);
        editor.SetText("");
        check(actions.changes == changesBeforeUnregister, "callback unregister");
#endif
        TextDiff diff;
        ImGui::DestroyContext();
        std::cout << "editor regression checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
