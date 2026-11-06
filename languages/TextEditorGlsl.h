//	TextEditor - A syntax highlighting text editor for Dear ImGui.
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	GetLanguageDefinitionGlsl
//

static const TextEditor::Language* GetLanguageDefinitionGlsl() {
	static bool initialized = false;
	static TextEditor::Language language;

	if (!initialized) {
		language.name = "GLSL";
		language.preprocess = '#';
		language.singleLineComment = "//";
		language.commentStart = "/*";
		language.commentEnd = "*/";
		language.hasSingleQuotedStrings = true;
		language.hasDoubleQuotedStrings = true;
		language.stringEscape = '\\';

		// source: https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html
		static const char* const keywords[] = {
			"atomic_uint", "attribute", "bool", "break", "buffer", "bvec2", "bvec3", "bvec4", "case", "centroid",
			"coherent", "const", "continue", "default", "discard", "dmat2", "dmat2x2", "dmat2x3", "dmat2x4", "dmat3",
			"dmat3x2", "dmat3x3", "dmat3x4", "dmat4", "dmat4x2", "dmat4x3", "dmat4x4", "do", "double", "dvec2", "dvec3",
			"dvec4", "else", "false", "flat", "float", "for", "highp", "if", "iimage1D", "iimage1DArray", "iimage2D",
			"iimage2DArray", "iimage2DMS", "iimage2DMSArray", "iimage2DRect", "iimage3D", "iimageBuffer", "iimageCube",
			"iimageCubeArray", "image1D", "image1DArray", "image2D", "image2DArray", "image2DMS", "image2DMSArray",
			"image2DRect", "image3D", "imageBuffer", "imageCube", "imageCubeArray", "in", "inout", "int", "invariant",
			"isampler1D", "isampler1DArray", "isampler2D", "isampler2DArray", "isampler2DMS", "isampler2DMSArray",
			"isampler2DRect", "isampler3D", "isamplerBuffer", "isamplerCube", "isamplerCubeArray", "ivec2", "ivec3",
			"ivec4", "layout", "lowp", "mat2", "mat2x2", "mat2x3", "mat2x4", "mat3", "mat3x2", "mat3x3", "mat3x4",
			"mat4", "mat4x2", "mat4x3", "mat4x4", "mediump", "noperspective", "out", "patch", "precise", "precision",
			"readonly", "restrict", "return", "sample", "sampler1D", "sampler1DArray", "sampler1DArrayShadow",
			"sampler1DShadow", "sampler2D", "sampler2DArray", "sampler2DArrayShadow", "sampler2DMS", "sampler2DMSArray",
			"sampler2DRect", "sampler2DRectShadow", "sampler2DShadow", "sampler3D", "samplerBuffer", "samplerCube",
			"samplerCubeArray", "samplerCubeArrayShadow", "samplerCubeShadow", "shared", "smooth", "struct", "subroutine",
			"switch", "true", "uimage1D", "uimage1DArray", "uimage2D", "uimage2DArray", "uimage2DMS", "uimage2DMSArray",
			"uimage2DRect", "uimage3D", "uimageBuffer", "uimageCube", "uimageCubeArray", "uint", "uniform", "usampler1D",
			"usampler1DArray", "usampler2D", "usampler2DArray", "usampler2DMS", "usampler2DMSArray", "usampler2DRect",
			"usampler3D", "usamplerBuffer", "usamplerCube", "usamplerCubeArray", "uvec2", "uvec3", "uvec4", "varying",
			"vec2", "vec3", "vec4", "void", "volatile", "while", "writeonly"
		};

		for (auto& keyword : keywords) { language.keywords.insert(keyword); }

		language.isPunctuation = isCStylePunctuation;
		language.getIdentifier = getCStyleIdentifier;
		language.getNumber = getCStyleNumber;
		initialized = true;
	}

	return &language;
}
