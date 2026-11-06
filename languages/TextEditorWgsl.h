//	TextEditor - A syntax highlighting text editor for Dear ImGui.
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//	Contributed by Yan Pujante (https://github.com/ypujante)


//
//	GetLanguageDefinitionWgsl
//

static const TextEditor::Language* GetLanguageDefinitionWgsl() {
	static bool initialized = false;
	static TextEditor::Language language;

	if (!initialized) {
		language.name = "WGSL";
		language.caseSensitive = true;
		language.singleLineComment = "//";
		language.commentStart = "/*";
		language.commentEnd = "*/";
		language.hasDoubleQuotedStrings = true;
		language.stringEscape = '\\';

		// keywords, attributes, types and functions extracted from https://www.w3.org/TR/WGSL
		static const char* const keywords[] = {
			"alias", "break", "case", "const", "const_assert", "continue", "continuing",
			"default", "diagnostic", "discard", "else", "enable", "false", "fn", "for",
			"if", "let", "loop", "override", "requires", "return", "struct", "switch",
			"true", "type", "var", "while",
			// reserved for future use
			"asm", "do", "enum", "goto", "inline", "noinline", "precedence",
			"read_only", "self", "sizeof", "static", "target", "typeof", "using"
		};

		for (auto& keyword : keywords) { language.keywords.insert(keyword); }

		static const char* const attributes[] = {
			"align", "binding", "builtin", "compute", "const", "fragment",
			"group", "id", "interpolate", "invariant", "location", "must_use",
			"size", "vertex", "workgroup_size"
		};

		for (auto& keyword : attributes) { language.keywords.insert(std::string("@") + keyword); }

		static const char* const types[] = {
			// scalar types
			"bool", "f16", "f32", "i32", "u32",
			// compound types
			"array", "atomic", "mat2x2", "mat2x3", "mat2x4", "mat3x2", "mat3x3",
			"mat3x4", "mat4x2", "mat4x3", "mat4x4", "ptr", "sampler", "sampler_comparison",
			"texture_1d", "texture_2d", "texture_2d_array", "texture_3d", "texture_cube",
			"texture_cube_array", "texture_multisampled_2d", "texture_storage_1d",
			"texture_storage_2d", "texture_storage_2d_array", "texture_storage_3d",
			"texture_depth_2d", "texture_depth_2d_array", "texture_depth_cube",
			"texture_depth_cube_array", "texture_depth_multisampled_2d", "vec2", "vec3", "vec4",
			// common aliases
			"vec2i", "vec3i", "vec4i", "vec2u", "vec3u", "vec4u", "vec2f", "vec3f", "vec4f",
			"vec2h", "vec3h", "vec4h", "mat2x2f", "mat2x3f", "mat2x4f", "mat3x2f", "mat3x3f",
			"mat3x4f", "mat4x2f", "mat4x3f", "mat4x4f", "mat2x2h", "mat3x3h", "mat4x4h"
		};

		for (auto& keyword : types) { language.keywords.insert(keyword); }

		static const char* const functions[] = {
			"abs", "acos", "acosh", "all", "any", "arrayLength", "asin", "asinh", "atan",
			"atanh", "atan2", "bitcast", "ceil", "clamp", "cos", "cosh", "countLeadingZeros",
			"countOneBits", "countTrailingZeros", "cross", "degrees", "determinant",
			"distance", "dot", "exp", "exp2", "faceForward", "firstLeadingBit",
			"firstTrailingBit", "floor", "fma", "fract", "frexp", "fwidth", "fwidthCoarse",
			"fwidthFine", "inverseSqrt", "ldexp", "length", "log", "log2", "max", "min",
			"mix", "modf", "normalize", "pow", "quantizeToF16", "radians", "reflect",
			"refract", "reverseBits", "round", "saturate", "select", "sign", "sin", "sinh",
			"smoothstep", "sqrt", "step", "storageBarrier", "tan", "tanh", "transpose",
			"trunc", "workgroupBarrier", "textureDimensions", "textureGather",
			"textureGatherCompare", "textureLoad", "textureSample", "textureSampleBias",
			"textureSampleCompare", "textureSampleCompareLevel", "textureSampleGrad",
			"textureSampleLevel", "textureStore"
		};

		for (auto& keyword : functions) { language.keywords.insert(keyword); }

		language.isPunctuation = isCStylePunctuation;
		language.getIdentifier = getCStyleIdentifier;
		language.getNumber = getCStyleNumber;
		initialized = true;
	}

	return &language;
}
