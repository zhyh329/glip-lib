#include "keywords.hpp"

	const char* GLSLKeyword[] =
	{
		"attribute\0",
		"const\0",
		"uniform\0",
		"varying\0",
		"layout\0",
		"centroid\0",
		"flat\0",
		"smooth\0",
		"noperspective\0",
		"patch\0",
		"sample\0",
		"break\0",
		"continue\0",
		"do\0",
		"for\0",
		"while\0",
		"switch\0",
		"case\0",
		"default\0",
		"if\0",
		"else\0",
		"subroutine\0",
		"in\0",
		"out\0",
		"inout\0",
		"float\0",
		"double\0",
		"int\0",
		"void\0",
		"bool\0",
		"true\0",
		"false\0",
		"invariant\0",
		"discard\0",
		"return\0",
		"mat2\0",
		"mat3\0",
		"mat4\0",
		"dmat2\0",
		"dmat3\0",
		"dmat4\0",
		"mat2x2\0",
		"mat2x3\0",
		"mat2x4\0",
		"dmat2x2\0",
		"dmat2x3\0",
		"dmat2x4\0",
		"mat3x2\0",
		"mat3x3\0",
		"mat3x4\0",
		"dmat3x2\0",
		"dmat3x3\0",
		"dmat3x4\0",
		"mat4x2\0",
		"mat4x3\0",
		"mat4x4\0",
		"dmat4x2\0",
		"dmat4x3\0",
		"dmat4x4\0",
		"vec2\0",
		"vec3\0",
		"vec4\0",
		"ivec2\0",
		"ivec3\0",
		"ivec4\0",
		"bvec2\0",
		"bvec3\0",
		"bvec4\0",
		"dvec2\0",
		"dvec3\0",
		"dvec4\0",
		"uint\0",
		"uvec2\0",
		"uvec3\0",
		"uvec4\0",
		"lowp\0",
		"mediump\0",
		"highp\0",
		"precision\0",
		"sampler1D\0",
		"sampler2D\0",
		"sampler3D\0",
		"samplerCube\0",
		"sampler1DShadow\0",
		"sampler2DShadow\0",
		"samplerCubeShadow\0",
		"sampler1DArray\0",
		"sampler2DArray\0",
		"sampler1DArrayShadow\0",
		"sampler2DArrayShadow\0",
		"isampler1D\0",
		"isampler2D\0",
		"isampler3D\0",
		"isamplerCube\0",
		"isampler1DArray\0",
		"isampler2DArray\0",
		"usampler1D\0",
		"usampler2D\0",
		"usampler3D\0",
		"usamplerCube\0",
		"usampler1DArray\0",
		"usampler2DArray\0",
		"sampler2DRect\0",
		"sampler2DRectShadow\0",
		"isampler2DRect\0",
		"usampler2DRect\0",
		"samplerBuffer\0",
		"isamplerBuffer\0",
		"usamplerBuffer\0",
		"sampler2DMS\0",
		"isampler2DMS\0",
		"usampler2DMS\0",
		"sampler2DMSArray\0",
		"isampler2DMSArray\0",
		"usampler2DMSArray\0",
		"samplerCubeArray\0",
		"samplerCubeArrayShadow\0",
		"isamplerCubeArray\0",
		"usamplerCubeArray\0",
		"struct\0",
		"common\0",
		"partition\0",
		"active\0",
		"asm\0",
		"class\0",
		"union\0",
		"enum\0",
		"typedef\0",
		"template\0",
		"this\0",
		"packed\0",
		"goto\0",
		"inline\0",
		"noinline\0",
		"volatile\0",
		"public\0",
		"static\0",
		"extern\0",
		"external\0",
		"interface\0",
		"long\0",
		"short\0",
		"half\0",
		"fixed\0",
		"unsigned\0",
		"superp\0",
		"input\0",
		"output\0",
		"hvec2\0",
		"hvec3\0",
		"hvec4\0",
		"fvec2\0",
		"fvec3\0",
		"fvec4\0",
		"sampler3DRect\0",
		"filter\0",
		"image1D\0",
		"image2D\0",
		"image3D\0",
		"imageCube\0",
		"iimage1D\0",
		"iimage2D\0",
		"iimage3D\0",
		"iimageCube\0",
		"uimage1D\0",
		"uimage2D\0",
		"uimage3D\0",
		"uimageCube\0",
		"image1DArray\0",
		"image2DArray\0",
		"iimage1DArray\0",
		"iimage2DArray\0",
		"uimage1DArray\0",
		"uimage2DArray\0",
		"image1DShadow\0",
		"image2DShadow\0",
		"image1DArrayShadow\0",
		"image2DArrayShadow\0",
		"imageBuffer\0",
		"iimageBuffer\0",
		"uimageBuffer\0",
		"sizeof\0",
		"cast\0",
		"namespace\0",
		"using\0",
		"row_major\0" // 185 lines
	};

	const char* GLSLFunction[] = 
	{
		"abs\0",
		"acos\0",
		"acosh\0",
		"all\0",
		"any\0",
		"asin\0",
		"asinh\0",
		"atan\0",
		"atanh\0",
		"atomicAdd\0",
		"atomicAnd\0",
		"atomicCompSwap\0",
		"atomicCounter\0",
		"atomicCounterDecrement\0",
		"atomicCounterIncrement\0",
		"atomicExchange\0",
		"atomicMax\0",
		"atomicMin\0",
		"atomicOr\0",
		"atomicXor\0",
		"barrier\0",
		"bitCount\0",
		"bitfieldExtract\0",
		"bitfieldInsert\0",
		"bitfieldReverse\0",
		"ceil\0",
		"clamp\0",
		"cos\0",
		"cosh\0",
		"cross\0",
		"degrees\0",
		"determinant\0",
		"dFdx\0",
		"dFdy\0",
		"distance\0",
		"dot\0",
		"EmitStreamVertex\0",
		"EmitVertex\0",
		"EndPrimitive\0",
		"EndStreamPrimitive\0",
		"equal\0",
		"exp\0",
		"exp2\0",
		"faceforward\0",
		"findLSB\0",
		"findMSB\0",
		"floatBitsToInt\0",
		"floatBitsToUint\0",
		"floor\0",
		"fma\0",
		"fract\0",
		"frexp\0",
		"fwidth\0",
		"greaterThan\0",
		"greaterThanEqual\0",
		"groupMemoryBarrier\0",
		"imageAtomicAdd\0",
		"imageAtomicAnd\0",
		"imageAtomicCompSwap\0",
		"imageAtomicExchange\0",
		"imageAtomicMax\0",
		"imageAtomicMin\0",
		"imageAtomicOr\0",
		"imageAtomicXor\0",
		"imageLoad\0",
		"imageSize\0",
		"imageStore\0",
		"imulExtended\0",
		"intBitsToFloat\0",
		"interpolateAtCentroid\0",
		"interpolateAtOffset\0",
		"interpolateAtSample\0",
		"inverse\0",
		"inversesqrt\0",
		"isinf\0",
		"isnan\0",
		"ldexp\0",
		"length\0",
		"lessThan\0",
		"lessThanEqual\0",
		"log\0",
		"log2\0",
		"matrixCompMult\0",
		"max\0",
		"memoryBarrier\0",
		"memoryBarrierAtomicCounter\0",
		"memoryBarrierBuffer\0",
		"memoryBarrierImage\0",
		"memoryBarrierShared\0",
		"min\0",
		"mix\0",
		"mod\0",
		"modf\0",
		"noise\0",
		"normalize\0",
		"not\0",
		"notEqual\0",
		"outerProduct\0",
		"packDouble2x32\0",
		"packHalf2x16\0",
		"packSnorm2x16\0",
		"packSnorm4x8\0",
		"packUnorm2x16\0",
		"packUnorm4x8\0",
		"pow\0",
		"radians\0",
		"reflect\0",
		"refract\0",
		"round\0",
		"roundEven\0",
		"sign\0",
		"sin\0",
		"sinh\0",
		"smoothstep\0",
		"sqrt\0",
		"step\0",
		"tan\0",
		"tanh\0",
		"texelFetch\0",
		"texelFetchOffset\0",
		"texture\0",
		"textureGather\0",
		"textureGatherOffset\0",
		"textureGatherOffsets\0",
		"textureGrad\0",
		"textureGradOffset\0",
		"textureLod\0",
		"textureLodOffset\0",
		"textureOffset\0",
		"textureProj\0",
		"textureProjGrad\0",
		"textureProjGradOffset\0",
		"textureProjLod\0",
		"textureProjLodOffset\0",
		"textureProjOffset\0",
		"textureQueryLevels\0",
		"textureQueryLod\0",
		"textureSize\0",
		"transpose\0",
		"trunc\0",
		"uaddCarry\0",
		"uintBitsToFloat\0",
		"umulExtended\0",
		"unpackDouble2x32\0",
		"unpackHalf2x16\0",
		"unpackSnorm2x16\0",
		"unpackSnorm4x8\0",
		"unpackUnorm2x16\0",
		"unpackUnorm4x8\0",
		"usubBorrow\0",
		"gl_ClipDistance\0",
		"gl_FragCoord\0",
		"gl_FragDepth\0",
		"gl_FrontFacing\0",
		"gl_GlobalInvocationID\0",
		"gl_InstanceID\0",
		"gl_InvocationID\0",
		"gl_Layer\0",
		"gl_LocalInvocationID\0",
		"gl_LocalInvocationIndex\0",
		"gl_NumSamples\0",
		"gl_NumWorkGroups\0",
		"gl_PatchVerticesIn\0",
		"gl_PointCoord\0",
		"gl_PointSize\0",
		"gl_Position\0",
		"gl_PrimitiveID\0",
		"gl_PrimitiveIDIn\0",
		"gl_SampleID\0",
		"gl_SampleMask\0",
		"gl_SampleMaskIn\0",
		"gl_SamplePosition\0",
		"gl_TessCoord\0",
		"gl_TessLevelInner\0",
		"gl_TessLevelOuter\0",
		"gl_VertexID\0",
		"gl_ViewportIndex\0",
		"gl_WorkGroupID\0",
		"gl_WorkGroupSize\0"
	};

