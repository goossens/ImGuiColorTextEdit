//	TextEditor - A syntax highlighting text editor for Dear ImGui.
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	GetLanguageDefinitionHlsl
//

static const TextEditor::Language* GetLanguageDefinitionHlsl() {
	static bool initialized = false;
	static TextEditor::Language language;

	if (!initialized) {
		language.name = "HLSL";
		language.preprocess = '#';
		language.singleLineComment = "//";
		language.commentStart = "/*";
		language.commentEnd = "*/";
		language.hasSingleQuotedStrings = true;
		language.hasDoubleQuotedStrings = true;
		language.stringEscape = '\\';

		static const char* const keywords[] = {
			"AppendStructuredBuffer", "asm", "asm_fragment", "BlendState", "bool", "break", "Buffer",
			"ByteAddressBuffer", "case", "cbuffer", "centroid", "class", "column_major", "compile",
			"compile_fragment", "CompileShader", "const", "continue", "ComputeShader", "ConsumeStructuredBuffer",
			"default", "DepthStencilState", "DepthStencilView", "discard", "do", "double", "DomainShader", "dword",
			"else", "export", "extern", "false", "float", "for", "fxgroup", "GeometryShader", "groupshared", "half",
			"Hullshader", "if", "in", "inline", "inout", "InputPatch", "int", "interface", "line", "lineadj",
			"linear", "LineStream", "matrix", "min16float", "min10float", "min16int", "min12int", "min16uint",
			"namespace", "nointerpolation", "noperspective", "NULL", "out", "OutputPatch", "packoffset",
			"pass", "pixelfragment", "PixelShader", "point", "PointStream", "precise", "RasterizerState",
			"RenderTargetView", "return", "register", "row_major", "RWBuffer", "RWByteAddressBuffer",
			"RWStructuredBuffer", "RWTexture1D", "RWTexture1DArray", "RWTexture2D", "RWTexture2DArray",
			"RWTexture3D", "sample", "sampler", "SamplerState", "SamplerComparisonState", "shared",
			"snorm", "stateblock", "stateblock_state", "static", "string", "struct", "switch", "StructuredBuffer",
			"tbuffer", "technique", "technique10", "technique11", "texture", "Texture1D", "Texture1DArray",
			"Texture2D", "Texture2DArray", "Texture2DMS", "Texture2DMSArray", "Texture3D", "TextureCube",
			"TextureCubeArray", "true", "typedef", "triangle", "triangleadj", "TriangleStream", "uint",
			"uniform", "unorm", "unsigned", "vector", "vertexfragment", "VertexShader", "void", "volatile", "while",
			"bool1", "bool2", "bool3", "bool4", "double1", "double2", "double3", "double4", "float1", "float2",
			"float3", "float4", "int1", "int2", "int3", "int4", "in", "out", "inout", "uint1", "uint2", "uint3",
			"uint4", "dword1", "dword2", "dword3", "dword4", "half1", "half2", "half3", "half4", "float1x1",
			"float2x1", "float3x1", "float4x1", "float1x2", "float2x2", "float3x2", "float4x2",
			"float1x3", "float2x3", "float3x3", "float4x3", "float1x4", "float2x4", "float3x4", "float4x4",
			"half1x1", "half2x1", "half3x1", "half4x1", "half1x2", "half2x2", "half3x2", "half4x2",
			"half1x3", "half2x3", "half3x3", "half4x3", "half1x4", "half2x4", "half3x4", "half4x4",
		};

		for (auto& keyword : keywords) { language.keywords.insert(keyword); }

		language.isPunctuation = isCStylePunctuation;
		language.getIdentifier = getCStyleIdentifier;
		language.getNumber = getCStyleNumber;
		initialized = true;
	}

	return &language;
}
