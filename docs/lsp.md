# Language Server Protocol (LSP) Bridge

The text editor does not directly interact with language servers as this
would overcomplicate the widget. An optional bridge is however available
as an addon contained in the **extras** folder. This feature is experimental
and certainly a work in progress.

The LspBridge class is base on a [LSP Framework](https://github.com/leon-bckl/lsp-framework)
which in turn depends on C++20. So if you want to use this bridge in your
application, you have to include the framework and ensure you are using
C++20. The example application does this and you can use it as a guide.
If C++20 or an extra dependency are a bridge (pun intended) too far for you,
feel free to come up with your own solution and contribute it back.

Right now, a Language Server is only used for autocomplete and rollover
help but in the future other services could be used.

To use the language server solution, do the following:

- Ensure your project uses C++20.
- Add the [LSP Framework](https://github.com/leon-bckl/lsp-framework).
- Include the **LspBridge.cpp** and **LspBridge.h** files in your project. Both are in the **extras** folder.
- Instantiate a **LspBridge** object.
- Use the **Start** and **Stop** methods to launch your desired language server (the example app launches **clangd** for C++).
- Use the **OpenDocument** and **CloseDocument** methods to track a document in a TextEditor instance. The bridge can track multiple documents in one instance so if you use it in an IDE like environment you only need one **LspBridge** object.
- Call **BeforeRender** and **AfterRender** methods before/after you call **Render** on the respective TextEditor.
- Everything else is done automatically for you.

Here is a simple snippet to use. As mentioned above, the example application uses this so you can have look there as well.

```c++

LspBridge bridge;

...
std::string rootPath = "/some/directory/as/the/root/of/your/project";
bridge.Start(rootPath, "clangd", {"--log=error", "--background-index"});

...
TextEditor editor;

...
std::string path = "/path/to/document/test.cpp"; // this is used as the document's identifier
bridge.OpenDocument(path, editor);

...
bridge.BeforeRender(path);
editor.Render("What a Good Looking Title");
bridge.AfterRender(path);

...
bridge.CloseDocument(path);

...
bridge.end();
```
