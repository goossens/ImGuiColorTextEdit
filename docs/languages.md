# Languages

The text editor has builtin support for a number of frequently used computer languages.
Currently the following languages are supported:

- AngelScript
- C
- C++
- C#
- GLSL
- HLSL
- Json
- Julia
- Lua
- Markdown
- Python
- SQL
- WGSL

By default, the TextEditor includes all included languages and makes it a runtime choice to use them or not.
If you want to slim down the TextEditor, you can change the included language definitions at compile time
based on the content of the TextEditorLanguages.h file.

To give you an idea how much you could save, when compiled on a Mac with Apple Silicone, using Clang in release mode
the TextEditor.o file is 535,912 bytes with all language definitions included and 393,224 bytes with no language
definitions included. This means the language definitions are about 25% of the compiled code.

Here are some simple scenarios:

## Include all included language definitions at compile time

Simply do nothing as this is the default. Just copy TextEditor.cpp, TextEditor.h, TextEditorLanguages.h and
the languages folder to your project as described in the README file.

## Include only some language definitions at compile time

You can deactivate/remove unwanted/unused definitions in the TextEditorLanguages.h file by commenting out
or removing the corresponding include and RegisterLanguage statements.

One important note is that some languages reuse parts of the C language definition so although you
might not need C directly, if you use AngelScript, C++, C#, GLSL, HLSL, Lua, Python, SQL or Wgsl, you
must also include the C definition to avoid link errors.

# Define language definitions outside the core

If you want to add your own languages, you must create new language definition files in a place where
the compiler can find that and add an include and RegisterLanguage statement for each added language
in the TextEditorLanguages.h file. Any of the language definitions in the languages folder can be used
as a template.

# Querying the language definition registry

The text editor has three global API calls to access the language definition registry:

- GetRegisteredLanguages - retrieve a vector of language names that are registered in the text editor.
- LookupLanguage - retrieve a language definition based on the language name (e.g. LookupLanguage("Lua")).
- LookupLanguageByExtension - retrieve a language definition based on file extension (e.g. LookupLanguageByExtension(".cpp")).
