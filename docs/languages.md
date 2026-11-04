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
- Lua
- Markdown
- Python
- SQL

By default, the TextEditor includes all builtin languages and makes it a runtime choice to use them or not.
If you want to slim down the TextEditor, you can request just one or more language definitions to be included at compile time
based on preprocessor definitions.

To give you an idea how much you could save, when compiled on a Mac with Apple Silicone, using Clang in release mode the TextEditor.o file is 535,912 bytes with all language definitions included and 393,224 bytes with no language definitions. This means the language definitions are about 25% of the compiled code.

Here are some simple scenarios:

## Include all builtin language definitions at compile time

Simply do nothing as this is the default.

## Include only some builtin language definitions at compile time

By defining one or more of the following preprocessor macros only those language definitions are included
(i.e. defining any of the below automatically deactivates all other language inclusions):

- TEXTEDITOR_INCLUDE_ANGELSCRIPT
- TEXTEDITOR_INCLUDE_C
- TEXTEDITOR_INCLUDE_CPP
- TEXTEDITOR_INCLUDE_CS
- TEXTEDITOR_INCLUDE_GLSL
- TEXTEDITOR_INCLUDE_HLSL
- TEXTEDITOR_INCLUDE_JSON
- TEXTEDITOR_INCLUDE_LUA
- TEXTEDITOR_INCLUDE_MARKDOWN
- TEXTEDITOR_INCLUDE_PYTHON
- TEXTEDITOR_INCLUDE_SQL

# Exclude all builtin language definitions at compile time

By defining the TEXTEDITOR_EXCLUDE_ALL preprocessor macro, no builtin languages will be included.

# Exclude some builtin language definitions at compile time

By defining one or more of the following preprocessor macros only those language definitions are excluded:

- TEXTEDITOR_EXCLUDE_ANGELSCRIPT
- TEXTEDITOR_EXCLUDE_C
- TEXTEDITOR_EXCLUDE_CPP
- TEXTEDITOR_EXCLUDE_CS
- TEXTEDITOR_EXCLUDE_GLSL
- TEXTEDITOR_EXCLUDE_HLSL
- TEXTEDITOR_EXCLUDE_JSON
- TEXTEDITOR_EXCLUDE_LUA
- TEXTEDITOR_EXCLUDE_MARKDOWN
- TEXTEDITOR_EXCLUDE_PYTHON
- TEXTEDITOR_EXCLUDE_SQL

# Language dependencies

Some languages (AngelScript, C++, C#, GLSL, HLSL, Lua, Python) use functions
provided by the C language definition. If any of these seven languages are included
without the C language, C will automatically be added.

# Define builtin language definitions outside the core

It would be impractical to add a language definition for every language under the
sun to the text editor. It possible however to add builtin languages from outside
the text editor core by using the **RegisterBuiltinLanguage** API. This is a global
API so it only has to be called once per program even if you have multiple
editor instances.

# Querying the list of builtin language definitions

The text editor has three global API calls to access the builtin language list:

- GetBuiltinLanguages - retrieve a vector of language names that are configured in the text editor. This includes builtin language configured above or languages that are defined externally and registered with RegisterBuiltinLanguage.
- GetBuiltinLanguage - retrieve a builtin language definition based on the language name (e.g. GetBuiltinLanguage("Lua")).
- GetBuiltinLanguageByExtension - retrieve a builtin language definition based on file extension (e.g. GetBuiltinLanguageByExtension(".cpp")).
