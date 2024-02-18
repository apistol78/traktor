/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Guid.h"
#include "Core/Platform.h"
#include "Core/Ref.h"
#include "Core/Io/Path.h"
#include "Core/Math/Color4f.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

// Clear some X11 defines.
#if defined(Always)
#	undef Always
#endif
#if defined(None)
#	undef None
#endif

namespace traktor::render
{

/*! \ingroup Render */
//@{

enum class PrecisionHint
{
	Undefined = 0,
	Low = 1,
	Medium = 2,
	High = 3
};

namespace RenderPriority
{
	enum
	{
		Setup = 1,
		Opaque = 2,
		PostOpaque = 4,
		AlphaBlend = 8,
		PostAlphaBlend = 16,
		Overlay = 32,
		All = (Setup | Opaque | PostOpaque | AlphaBlend | PostAlphaBlend | Overlay)
	};
}

enum class CullMode
{
	Never = 0,
	ClockWise = 1,
	CounterClockWise = 2
};

enum class BlendOperation
{
	Add = 0,
	Subtract = 1,
	ReverseSubtract = 2,
	Min = 3,
	Max = 4
};

enum class BlendFactor
{
	One = 0,
	Zero = 1,
	SourceColor = 2,
	OneMinusSourceColor = 3,
	DestinationColor = 4,
	OneMinusDestinationColor = 5,
	SourceAlpha = 6,
	OneMinusSourceAlpha = 7,
	DestinationAlpha = 8,
	OneMinusDestinationAlpha = 9
};

namespace ColorWrite
{
	enum
	{
		Red = 1,
		Green = 2,
		Blue = 4,
		Alpha = 8,
		RGB = ColorWrite::Red | ColorWrite::Green | ColorWrite::Blue,
		RGBA = ColorWrite::Red | ColorWrite::Green | ColorWrite::Blue | ColorWrite::Alpha
	};
}

enum class CompareFunction
{
	Always = 0,
	Never = 1,
	Less = 2,
	LessEqual = 3,
	Greater = 4,
	GreaterEqual = 5,
	Equal = 6,
	NotEqual = 7,
	None = 8
};

enum class StencilOperation
{
	Keep = 0,
	Zero = 1,
	Replace = 2,
	IncrementSaturate = 3,
	DecrementSaturate = 4,
	Invert = 5,
	Increment = 6,
	Decrement = 7
};

enum class Filter
{
	Point = 0,
	Linear = 1
};

enum class Address
{
	Wrap = 0,
	Mirror = 1,
	Clamp = 2,
	Border = 3
};

/*! Render state. */
struct RenderState
{
	CullMode cullMode = CullMode::CounterClockWise;
	bool blendEnable = false;
	BlendOperation blendColorOperation = BlendOperation::Add;
	BlendFactor blendColorSource = BlendFactor::One;
	BlendFactor blendColorDestination = BlendFactor::Zero;
	BlendOperation blendAlphaOperation = BlendOperation::Add;
	BlendFactor blendAlphaSource = BlendFactor::One;
	BlendFactor blendAlphaDestination = BlendFactor::One;
	uint32_t colorWriteMask = ColorWrite::RGBA;
	bool depthEnable = true;
	bool depthWriteEnable = true;
	CompareFunction depthFunction = CompareFunction::LessEqual;
	bool alphaTestEnable = false;
	CompareFunction alphaTestFunction = CompareFunction::Less;
	int32_t alphaTestReference = 128;
	bool alphaToCoverageEnable = false;
	bool wireframe = false;
	bool stencilEnable = false;
	StencilOperation stencilFail = StencilOperation::Keep;
	StencilOperation stencilZFail = StencilOperation::Keep;
	StencilOperation stencilPass = StencilOperation::Keep;
	CompareFunction stencilFunction = CompareFunction::Always;
	uint32_t stencilReference = 0x00000000;
	uint32_t stencilMask = 0xffffffff;
};

/*! Sampler state. */
struct SamplerState
{
	Filter minFilter = Filter::Linear;
	Filter mipFilter = Filter::Linear;
	Filter magFilter = Filter::Linear;
	Address addressU = Address::Wrap;
	Address addressV = Address::Wrap;
	Address addressW = Address::Wrap;
	CompareFunction compare = CompareFunction::None;
	float mipBias = 0.0f;
	bool ignoreMips = false;
	bool useAnisotropic = false;
};

/*! Render view event types. */
enum class RenderEventType
{
	Close = 1,
	Resize = 2,
	ToggleFullScreen = 3,
	SetWindowed = 4,
	SetFullScreen = 5,
	Lost = 6
};

/*! Buffer usage flags. */
enum BufferUsage
{
	BuVertex = 1,
	BuIndex = 2,
	BuStructured = 4,
	BuIndirect = 8
};

/*! Clear target flags. */
enum ClearFlag
{
	CfColor		= 1,	//!< Clear color buffer.
	CfDepth		= 2,	//!< Clear depth buffer.
	CfStencil	= 4		//!< Clear stencil buffer.
};

/*! Vertex element data usage. */
enum class DataUsage
{
	Position	= 0,	//!< Positions
	Normal		= 1,	//!< Normals
	Tangent		= 2,	//!< Tangents
	Binormal	= 3,	//!< Bi-normals
	Color		= 4,	//!< Colors
	Custom		= 5		//!< Custom, ex. texture coordinates etc.
};

/*! Vertex element data type. */
enum DataType
{
	DtFloat1	= 0,	//!< Single float.
	DtFloat2	= 1,	//!< 2 floats.
	DtFloat3	= 2,	//!< 3 floats.
	DtFloat4	= 3,	//!< 4 floats.
	DtByte4		= 4,	//!< 4 unsigned bytes.
	DtByte4N	= 5,	//!< 4 unsigned bytes, normalized to 0 - 1.
	DtShort2	= 6,	//!< 2 signed shorts.
	DtShort4	= 7,	//!< 4 signed shorts.
	DtShort2N	= 8,	//!< 2 signed shorts, normalized to -1 - 1.
	DtShort4N	= 9,	//!< 4 signed shorts, normalized to -1 - 1.
	DtHalf2		= 10,	//!< 2 half precision floats.
	DtHalf4		= 11,	//!< 4 half precision floats.
	DtInteger1  = 12,	//!< Single integer.
	DtInteger2  = 13,	//!< 2 integers.
	DtInteger3  = 14,	//!< 3 integers.
	DtInteger4  = 15	//!< 4 integers.
};

/*! Shader parameter type. */
enum class ParameterType
{
	Scalar       = 0,	//!< Scalar parameter.
	Vector       = 1,	//!< Vector parameter.
	Matrix       = 2,	//!< Matrix parameter.
	Texture2D    = 3,	//!< 2D texture parameter.
	Texture3D    = 4,	//!< 3D texture parameter.
	TextureCube  = 5,	//!< Cube texture parameter.
	StructBuffer = 6,	//!< Struct buffer parameter.
	Image2D      = 7,
	Image3D      = 8,
	ImageCube    = 9
};

/*! Shader parameter update frequency. */
enum class UpdateFrequency
{
	Once		= 0,	//!< Once per life time.
	Frame		= 1,	//!< Once per frame.
	Draw		= 2		//!< Per draw call.
};

/*! Index type. */
enum class IndexType
{
	Void	= 0,	//!< No type.
	UInt16	= 1,	//!< Unsigned 16 bit indices.
	UInt32	= 2		//!< Unsigned 32 bit indices.
};

/*! Texture type. */
enum TextureType
{
	TtInvalid = 0,
	Tt2D = 1,
	Tt3D = 2,
	TtCube = 3
};

/*! Texture data format. */
enum TextureFormat
{
	TfInvalid = 0,

	TfR8 = 1,
	TfR8G8B8A8 = 2,
	TfR5G6B5 = 3,
	TfR5G5B5A1 = 4,
	TfR4G4B4A4 = 5,
	TfR10G10B10A2 = 6,

	/*! \name Floating point formats. */
	//@{

	TfR16G16B16A16F = 10,
	TfR32G32B32A32F = 11,
	TfR16G16F = 12,
	TfR32G32F = 13,
	TfR16F = 14,
	TfR32F = 15,
	TfR11G11B10F = 16,

	//@}

	/*! \name Compressed texture formats. */
	//@{

	TfDXT1 = 30,	// BC1
	TfDXT2 = 31,	// BC2 (premul alpha)
	TfDXT3 = 32,	// BC2
	TfDXT4 = 33,	// BC3 (premul alpha)
	TfDXT5 = 34,	// BC3
	TfBC6HU = 35,	// BC6H (unsigned)
	TfBC6HS = 36,	// BC6H (signed)
	TfPVRTC1 = 40,	// 4bpp, no alpha
	TfPVRTC2 = 41,	// 2bpp, no alpha
	TfPVRTC3 = 42,	// 4bpp, alpha
	TfPVRTC4 = 43,	// 2bpp, alpha
	TfETC1 = 44,
	TfASTC4x4 = 45,
	TfASTC8x8 = 46,
	TfASTC10x10 = 47,
	TfASTC12x12 = 48,
	TfASTC4x4F = 49,
	TfASTC8x8F = 50,
	TfASTC10x10F = 51,
	TfASTC12x12F = 52
	//@}
};

/*! Primitive topology type. */
enum class PrimitiveType
{
	Points = 0,
	LineStrip = 1,
	Lines = 2,
	TriangleStrip = 3,
	Triangles = 4
};

/*! Stage */
enum class Stage : uint32_t
{
	Invalid = 0,
	Vertex = 1,
	Fragment = 2,
	Compute = 4,
	Indirect = 8
};

/*! Stage */
constexpr Stage operator | (const Stage lh, const Stage rh) { return (Stage)((uint32_t)lh | (uint32_t)rh); }
constexpr Stage operator & (const Stage lh, const Stage rh) { return (Stage)((uint32_t)lh & (uint32_t)rh); }

/*! Render view event. */
struct RenderEvent
{
	RenderEventType type;
	union
	{
		// ReResize
		struct
		{
			int32_t width;
			int32_t height;
		}
		resize;
	};
};

/*! Render system statistics. */
struct RenderSystemStatistics
{
	// Memory usage.
	uint64_t memoryAvailable = 0;
	uint64_t memoryUsage = 0;
	uint64_t allocationCount = 0;

	// Number of alive resources.
	uint32_t buffers = 0;
	uint32_t simpleTextures = 0;
	uint32_t cubeTextures = 0;
	uint32_t volumeTextures = 0;
	uint32_t renderTargetSets = 0;
	uint32_t programs = 0;
};

/*! Render view statistics. */
struct RenderViewStatistics
{
	// Last frame.
	uint32_t passCount = 0;
	uint32_t drawCalls = 0;
	uint32_t primitiveCount = 0;
};

/*! Render view port. */
struct Viewport
{
	int32_t left = 0;
	int32_t top = 0;
	int32_t width = 0;
	int32_t height = 0;
	float nearZ = 0.0f;
	float farZ = 1.0f;

	Viewport() = default;

	Viewport(int32_t l, int32_t t, int32_t w, int32_t h, float nz, float fz)
	:	left(l)
	,	top(t)
	,	width(w)
	,	height(h)
	,	nearZ(nz)
	,	farZ(fz)
	{
	}
};

/*! Display mode structure. */
struct DisplayMode
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint16_t colorBits = 0;
	float refreshRate = 0.0f;
};

/*! Vendor type. */
enum class AdapterVendorType
{
	Unknown = 0,
	NVidia = 1,
	AMD = 2,
	Intel = 3,
	PowerVR = 4
};

/*! Render system information. */
struct RenderSystemInformation
{
	AdapterVendorType vendor = AdapterVendorType::Unknown;
	uint32_t dedicatedMemoryTotal = 0;
	uint32_t sharedMemoryTotal = 0;
	uint32_t dedicatedMemoryAvailable = 0;
	uint32_t sharedMemoryAvailable = 0;
};

/*! Descriptor for render system. */
struct RenderSystemDesc
{
	class IRenderSystem* capture = nullptr;
	SystemApplication sysapp;
	int32_t adapter = -1;
	float mipBias = 0.0f;
	int32_t maxAnisotropy = 1;
	bool validation = false;
	bool programCache = true;
	bool verbose = false;
};

/*! Descriptor for render views. */
struct RenderViewDesc
{
	uint16_t depthBits = 0;
	uint16_t stencilBits = 0;
	uint32_t multiSample = 0;
	float multiSampleShading = 0.0f;
	int32_t waitVBlanks = 0;
};

/*! Descriptor for default render views. */
struct RenderViewDefaultDesc : public RenderViewDesc
{
	uint32_t display = 0;
	DisplayMode displayMode;
	bool fullscreen = true;
	std::wstring title;
};

/*! Descriptor for embedded render views. */
struct RenderViewEmbeddedDesc : public RenderViewDesc
{
	SystemWindow syswin;
};

/*! Immutable texture data. */
struct TextureInitialData
{
	const void* data = nullptr;
	uint32_t pitch = 0;
	uint32_t slicePitch = 0;
};

/*! Descriptor for simple textures. */
struct SimpleTextureCreateDesc
{
	int32_t width = 0;
	int32_t height = 0;
	int32_t mipCount = 0;
	TextureFormat format = TfInvalid;
	bool sRGB = false;
	bool immutable = false;
	bool shaderStorage = false;
	TextureInitialData initialData[16];
};

/*! Descriptor for cube textures. */
struct CubeTextureCreateDesc
{
	int32_t side = 0;
	int32_t mipCount = 0;
	TextureFormat format = TfInvalid;
	bool sRGB = false;
	bool immutable = false;
	bool shaderStorage = false;
	TextureInitialData initialData[16 * 6];
};

/*! Descriptor for volume textures. */
struct VolumeTextureCreateDesc
{
	int32_t width = 0;
	int32_t height = 0;
	int32_t depth = 0;
	int32_t mipCount = 0;
	TextureFormat format = TfInvalid;
	bool sRGB = false;
	bool immutable = false;
	bool shaderStorage = false;
	TextureInitialData initialData[16];
};

/*! Descriptor for render target. */
struct RenderTargetCreateDesc
{
	TextureFormat format = TfInvalid;	/*< Render target pixel format. */
	bool sRGB = false;
};

class IRenderTargetSet;

/*! Descriptor for render target sets. */
struct RenderTargetSetCreateDesc
{
	enum { MaxTargets = 8 };

	int32_t count = 0;							/*!< Number of render targets in set; max 4 targets allowed. */
	int32_t width = 0;							/*!< Width of render targets. */
	int32_t height = 0;							/*!< Height of render targets. */
	int32_t multiSample = 0;					/*!< Number of samples; 0 no multisample. */
	bool createDepthStencil = false;			/*!< Attach depth/stencil buffer; shared among all targets. */
	bool usingPrimaryDepthStencil = false;		/*!< Share primary depth/stencil buffer; shared among all targets. */
	bool usingDepthStencilAsTexture = false;	/*!< Will be using depth/stencil buffer as a texture input of shaders. */
	bool usingDepthStencilAsStorage = false;	/*!< Will be using depth/stencil buffer as storage input or output of compute shader. */
	bool ignoreStencil = true;					/*!< Ignoring stencil; stencil isn't used in rendering. */
	bool generateMips = false;					/*!< Generate complete mip-chain after target been renderered onto. */
	RenderTargetCreateDesc targets[MaxTargets];	/*!< Descriptor for each target. */
};

/*! Target transfer flags. */
enum TransferFlags
{
	TfNone = 0,
	TfColor = 1,
	TfDepth = 2,
	TfAll = TfColor | TfDepth
};

/*! Clear parameters. */
struct Clear
{
	uint32_t mask = 0;		//!< Combination of ClearFlags.
	Color4f colors[RenderTargetSetCreateDesc::MaxTargets];	//!< Clear color values; must be one color for each bound target.
	float depth = 1.0f;		//!< Clear depth value.
	int32_t stencil = 0;	//!< Clear stencil value.
};

/*! Draw primitives. */
struct Primitives
{
	PrimitiveType type;
	bool indexed;
	uint32_t offset;
	uint32_t count;
	uint32_t minIndex;
	uint32_t maxIndex;

	Primitives() = default;

	Primitives(PrimitiveType type_, uint32_t offset_, uint32_t count_)
	{
		setNonIndexed(type_, offset_, count_);
	}

	Primitives(PrimitiveType type_, uint32_t offset_, uint32_t count_, uint32_t minIndex_, uint32_t maxIndex_)
	{
		setIndexed(type_, offset_, count_, minIndex_, maxIndex_);
	}

	inline void setNonIndexed(PrimitiveType type_, uint32_t offset_, uint32_t count_)
	{
		type = type_;
		indexed = false;
		offset = offset_;
		count = count_;
		minIndex = 0;
		maxIndex = 0;
	}

	inline void setIndexed(PrimitiveType type_, uint32_t offset_, uint32_t count_, uint32_t minIndex_, uint32_t maxIndex_)
	{
		type = type_;
		indexed = true;
		offset = offset_;
		count = count_;
		minIndex = minIndex_;
		maxIndex = maxIndex_;
	}

	inline uint32_t getVertexCount() const
	{
		const static uint32_t c_primitiveMul[] = { 1, 0, 2, 1, 3 };
		const static uint32_t c_primitiveAdd[] = { 0, 0, 0, 2, 0 };
		return count * c_primitiveMul[(int32_t)type] + c_primitiveAdd[(int32_t)type];
	}
};

/*! Copy region. */
struct Region
{
	int32_t x;
	int32_t y;
	int32_t z;		//!< Also cubemap side.
	int32_t mip;
	int32_t width;
	int32_t height;
	int32_t depth;
};

/*! Indirect draw structure. */
struct IndirectDraw
{
	uint32_t vertexCount;
	uint32_t instanceCount;
	int32_t firstVertex;
	uint32_t firstInstance;
};

/*! Indirect draw structure, indexed draw. */
struct IndexedIndirectDraw
{
	uint32_t indexCount;
	uint32_t instanceCount;
	uint32_t firstIndex;
	int32_t vertexOffset;
	uint32_t firstInstance;
};

/*! Shader parameter handle. */
typedef uint32_t handle_t;

/*! Return handle from parameter name.
 *
 * \param name Parameter name.
 * \return Parameter handle.
 */
handle_t T_DLLCLASS getParameterHandle(const std::wstring& name);

/*! Get name of handle.
 *
 * Useful for debugging purposes only,
 * since resolving name from handle is very slow.
 *
 * \param handle Parameter handle.
 * \return Parameter name.
 */
std::wstring T_DLLCLASS getParameterName(handle_t handle);

/*! Synthesize parameter name from index.
 *
 * \param index Texture reference index.
 * \return Parameter name.
 */
std::wstring T_DLLCLASS getParameterNameFromTextureReferenceIndex(int32_t index);

/*! Synthesize parameter handle from index.
 *
 * \param index Texture reference index.
 * \return Parameter handle.
 */
handle_t T_DLLCLASS getParameterHandleFromTextureReferenceIndex(int32_t index);

/*! Return human readable description of data usage. */
std::wstring T_DLLCLASS getDataUsageName(DataUsage usage);

/*! Return human readable description of data type. */
std::wstring T_DLLCLASS getDataTypeName(DataType dataType);

/*! Return number of elements from data type. */
uint32_t T_DLLCLASS getDataElementCount(DataType dataType);

/*! Return human readable description of texture format. */
std::wstring T_DLLCLASS getTextureFormatName(TextureFormat format);

/*! Return byte size from a texture format.
 *
 * \param format Texture format.
 * \return Byte size of block.
 */
uint32_t T_DLLCLASS getTextureBlockSize(TextureFormat format);

/*! Texture block denominator, i.e. block dimension (1x1 or 4x4 etc).
 *
 * \param format Texture format.
 * \return Block denominator.
 */
uint32_t T_DLLCLASS getTextureBlockDenom(TextureFormat format);

/*! Get texture mip size.
 *
 * \param textureSize Size of texture in pixels.
 * \param mipLevel Mip level.
 * \return Mip level size in pixels.
 */
uint32_t T_DLLCLASS getTextureMipSize(uint32_t textureSize, uint32_t mipLevel);

/*! Calculate pitch in bytes from format and width.
 *
 * \param format Texture format.
 * \param textureWidth Width of texture in pixels.
 * \return Texture pitch in bytes.
 */
uint32_t T_DLLCLASS getTextureRowPitch(TextureFormat format, uint32_t textureWidth);

/*! Calculate pitch in bytes from format and width.
*
* \param format Texture format.
* \param textureWidth Width of texture in pixels.
* \param mipLevel Mip level.
* \return Texture pitch in bytes.
*/
uint32_t T_DLLCLASS getTextureRowPitch(TextureFormat format, uint32_t textureWidth, uint32_t mipLevel);

/*! Calculate pitch in bytes from format and width, pitch of an entire mip.
 *
 * \param format Texture format.
 * \param textureWidth Width of texture in pixels.
 * \param textureHeight Height of texture in pixels.
 * \return Mip pitch in bytes.
 */
uint32_t T_DLLCLASS getTextureMipPitch(TextureFormat format, uint32_t textureWidth, uint32_t textureHeight);

/*! Calculate pitch in bytes from format and width, pitch of an entire mip.
 *
 * \param format Texture format.
 * \param textureWidth Width of texture in pixels.
 * \param textureHeight Height of texture in pixels.
 * \param mipLevel Mip level.
 * \return Mip pitch in bytes.
 */
uint32_t T_DLLCLASS getTextureMipPitch(TextureFormat format, uint32_t textureWidth, uint32_t textureHeight, uint32_t mipLevel);

/*! Calculate texture size; assuming continious layout.
 *
 * \param format Texture format.
 * \param textureWidth Width of texture in pixels.
 * \param textureHeight Height of texture in pixels.
 * \param mipLevels Number of mip levels.
 * \return Texture size in bytes.
 */
uint32_t T_DLLCLASS getTextureSize(TextureFormat format, uint32_t textureWidth, uint32_t textureHeight, uint32_t mipLevels);

/*! Estimate memory usage of a render target set.
 *
 * \note
 * This is purely for debugging purposes and might
 * be grossly inaccurate. Alignment and other system
 * specifics are ignored and everything is assumed
 * to be perfectly packed.
 *
 * \param rtscd Render target set create description.
 * \return Estimate of how much memory such target set use.
 */
uint32_t T_DLLCLASS getTargetSetMemoryEstimate(const RenderTargetSetCreateDesc& rtscd);

/*! Automatically resolved handles from literal. */
class T_DLLCLASS Handle
{
public:
	Handle() = default;

	explicit Handle(handle_t id)
	:	m_id(id)
	{
	}

	explicit Handle(const wchar_t* const name)
	{
		m_id = getParameterHandle(name);
	}

	Handle& operator = (handle_t id)
	{
		m_id = id;
		return *this;
	}

	Handle& operator = (const wchar_t* const name)
	{
		m_id = getParameterHandle(name);
		return *this;
	}

	operator handle_t () const
	{
		return m_id;
	}

private:
	handle_t m_id = 0;
};

//@}

}
