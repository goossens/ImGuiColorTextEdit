# Word Wrap

When word wrap mode is activated (see **SetWordWrapEnabled** API call),
lines are soft-wrapped within the available space based on a two part process.
First line break opportunities are identified and then a greedy algorithm
is used to fill the available space based on those identified opportunities.
If there are no break opportunities in a line, the text is rudely truncated to
fit within the space.

The first process is configurable and offers two algorithms. The first and simple
algorithm works on a set of glyphs can that you can break on before or after.
This is fast and seems to work fine when editing code. Theoretically, defaults
could be provided based on the selected language but given that people have
religious arguments about formatting code, an API is available to customize this.

The second algorithm is based on the Unicode Line Break Algorithm documented in
[Annex #14 of the Unicode standard](https://www.unicode.org/reports/tr14).
This algorithm is expressed as a set of rules and the TextEditor allows you
to (de)activate individual rules to meet your needs. The example app shows how
to configure the unicode algorithm to be more suitable for code editing
(versus text editing).

The line break process can be configured through the **SetLineBreakConfig** API call
which provides a reference to a LineBreakConfig struct.

```c++
// configuration for line breaking algorithm used when word wrap is active
struct LineBreakConfig {
	// wrap mode (false = simple mode, true = unicode line break mode)
	bool useUnicodeAnnex14 = false;

	// simple mode options (strings of UTF-8 encoded glyphs)
	std::string breakAfter = " \t{[(";
	std::string breakBefore = ".";

	// unicode line breaking options
	// based on the unicode standard annex #14 which identifies break
	// opportunities expressed as rules which can be (de)activated below
	// see https://www.unicode.org/reports/tr14 for details
	bool lb2 = true;
	bool lb3 = true;
	bool lb4 = true;
	bool lb5 = true;
	bool lb6 = true;
	bool lb7 = true;
	bool lb8 = true;
	bool lb8a = true;
	bool lb9 = true;
	bool lb10 = true;
	bool lb11 = true;
	bool lb12 = true;
	bool lb12a = true;
	bool lb13 = true;
	bool lb14 = true;
	bool lb15a = true;
	bool lb15b = true;
	bool lb15c = true;
	bool lb15d = true;
	bool lb16 = true;
	bool lb17 = true;
	bool lb18 = true;
	bool lb19 = true;
	bool lb19a = true;
	bool lb20 = true;
	bool lb20a = true;
	bool lb21 = true;
	bool lb21a = true;
	bool lb21b = true;
	bool lb22 = true;
	bool lb23 = true;
	bool lb23a = true;
	bool lb24 = true;
	bool lb25 = true;
	bool lb26 = true;
	bool lb27 = true;
	bool lb28 = true;
	bool lb28a = true;
	bool lb29 = true;
	bool lb30 = true;
	bool lb30a = true;
	bool lb30b = true;
};
```

### Notes:

* The editor always renders all glyphs so users can select and/or edit them. This means that whitespace at a soft break point is visible unlike in some other editors where they are suppressed.
* On a soft break, an automatic indent is applied to line up wrapped text with the previous line. This indent is only a visual feature and whitespaces are not inserted into the text/