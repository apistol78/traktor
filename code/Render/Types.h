#ifndef traktor_render_Types_H
#define traktor_render_Types_H

#include "Core/Config.h"
#include "Core/Guid.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \ingroup Render */
//@{

enum RenderPriority
{
	RpSetup = 1,
	RpOpaque = 2,
	RpPostOpaque = 4,
	RpAlphaBlend = 8,
	RpPostAlphaBlend = 16,
	RpOverlay = 32,
	RpAll = (RpSetup | RpOpaque | RpPostOpaque | RpAlphaBlend | RpPostAlphaBlend | RpOverlay)
};

enum CullMode
{
	CmNever,
	CmClockWise,
	CmCounterClockWise
};

enum BlendOperation
{
	BoAdd,
	BoSubtract,
	BoReverseSubtract,
	BoMin,
	BoMax
};

enum BlendFactor
{
	BfOne,
	BfZero,
	BfSourceColor,
	BfOneMinusSourceColor,
	BfDestinationColor,
	BfOneMinusDestinationColor,
	BfSourceAlpha,
	BfOneMinusSourceAlpha,
	BfDestinationAlpha,
	BfOneMinusDestinationAlpha
};

enum ColorWrite
{
	CwRed = 1,
	CwGreen = 2,
	CwBlue = 4,
	CwAlpha = 8
};

enum CompareFunction
{
	CfAlways,
	CfNever,
	CfLess,
	CfLessEqual,
	CfGreater,
	CfGreaterEqual,
	CfEqual,
	CfNotEqual
};

enum StencilOperation
{
	SoKeep,
	SoZero,
	SoReplace,
	SoIncrementSaturate,
	SoDecrementSaturate,
	SoInvert,
	SoIncrement,
	SoDecrement
};

/*! \brief Render state. */
struct RenderState
{
	CullMode cullMode;
	bool blendEnable;
	BlendOperation blendOperation;
	BlendFactor blendSource;
	BlendFactor blendDestination;
	uint32_t colorWriteMask;
	bool depthEnable;
	bool depthWriteEnable;
	CompareFunction depthFunction;
	bool alphaTestEnable;
	CompareFunction alphaTestFunction;
	int32_t alphaTestReference;
	bool alphaToCoverageEnable;
	bool wireframe;
	bool stencilEnable;
	StencilOperation stencilFail;
	StencilOperation stencilZFail;
	StencilOperation stencilPass;
	CompareFunction stencilFunction;
	uint32_t stencilReference;
	uint32_t stencilMask;

	RenderState()
	:	cullMode(CmCounterClockWise)
	,	blendEnable(false)
	,	blendOperation(BoAdd)
	,	blendSource(BfOne)
	,	blendDestination(BfZero)
	,	colorWriteMask(CwRed | CwGreen | CwBlue | CwAlpha)
	,	depthEnable(true)
	,	depthWriteEnable(true)
	,	depthFunction(CfLessEqual)
	,	alphaTestEnable(false)
	,	alphaTestFunction(CfLess)
	,	alphaTestReference(128)
	,	alphaToCoverageEnable(false)
	,	wireframe(false)
	,	stencilEnable(false)
	,	stencilFail(SoKeep)
	,	stencilZFail(SoKeep)
	,	stencilPass(SoKeep)
	,	stencilFunction(CfAlways)
	,	stencilReference(0x00000000)
	,	stencilMask(0xffffffff)
	{
	}
};

/*! \brief Render view event types. */
enum RenderEventType
{
	ReClose = 1,
	ReResize = 2,
	ReToggleFullScreen = 3
};

/*! \brief Clear target flags. */
enum ClearFlag
{
	CfColor		= 1,	//!< Clear color buffer.
	CfDepth		= 2,	//!< Clear depth buffer.
	CfStencil	= 4		//!< Clear stencil buffer.
};

/*! \brief Vertex element data usage. */
enum DataUsage
{
	DuPosition	= 0,	//!< Positions
	DuNormal	= 1,	//!< Normals
	DuTangent	= 2,	//!< Tangents
	DuBinormal	= 3,	//!< Bi-normals
	DuColor		= 4,	//!< Colors
	DuCustom	= 5		//!< Custom, ex. texture coordinates etc.
};

/*! \brief Vertex element data type. */
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
	DtHalf4		= 11	//!< 4 half precision floats.
};

/*! \brief Shader parameter type. */
enum ParameterType
{
	PtScalar	= 0,	//!< Scalar parameter.
	PtVector	= 1,	//!< Vector parameter.
	PtMatrix	= 2,	//!< Matrix parameter.
	PtTexture2D	= 3,	//!< 2D texture parameter.
	PtTexture3D	= 4,	//!< 3D texture parameter.
	PtTextureCube = 5	//!< Cube texture parameter.
};

/*! \brief Shader parameter update frequency. */
enum UpdateFrequency
{
	UfOnce		= 0,	//!< Once per life time.
	UfFrame		= 1,	//!< Once per frame.
	UfDraw		= 2		//!< Per draw call.
};

/*! \brief Index type. */
enum IndexType
{
	ItUInt16,			//!< Unsigned 16 bit indices.
	ItUInt32			//!< Unsigned 32 bit indices.
};

/*! \brief Texture type. */
enum TextureType
{
	TtInvalid = 0,
	Tt2D = 1,
	Tt3D = 2,
	TtCube = 3
};

/*! \brief Texture data format. */
enum TextureFormat
{
	TfInvalid = 0,

	TfR8 = 1,
	TfR8G8B8A8 = 2,
	TfR5G6B5 = 3,
	TfR5G5B5A1 = 4,
	TfR4G4B4A4 = 5,

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

	TfDXT1 = 30,
	TfDXT2 = 31,
	TfDXT3 = 32,
	TfDXT4 = 33,
	TfDXT5 = 34,
	TfPVRTC1 = 40,	// 4bpp, no alpha
	TfPVRTC2 = 41,	// 2bpp, no alpha
	TfPVRTC3 = 42,	// 4bpp, alpha
	TfPVRTC4 = 43,	// 2bpp, alpha
	TfETC1 = 44

	//@}
};

/*! \brief Primitive topology type. */
enum PrimitiveType
{
	PtPoints,
	PtLineStrip,
	PtLines,
	PtTriangleStrip,
	PtTriangles
};

/*! \brief Eye in stereoscopic rendering */
enum EyeType
{
	EtCyclop,	//< Not using stereoscopic rendering.
	EtLeft,
	EtRight,
};

/*! \brief Render view event. */
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

/*! \brief Render system statistics. */
struct RenderSystemStatistics
{
	// Number of alive resources.
	uint32_t vertexBuffers;
	uint32_t simpleTextures;
	uint32_t cubeTextures;
	uint32_t volumeTextures;
	uint32_t renderTargetSets;
	uint32_t programs;

	RenderSystemStatistics()
	:	vertexBuffers(0)
	,	simpleTextures(0)
	,	cubeTextures(0)
	,	volumeTextures(0)
	,	renderTargetSets(0)
	,	programs(0)
	{
	}
};

/*! \brief Render view statistics. */
struct RenderViewStatistics
{
	// Last frame.
	uint32_t drawCalls;
	uint32_t primitiveCount;

	RenderViewStatistics()
	:	drawCalls(0)
	,	primitiveCount(0)
	{
	}
};

/*! \brief Render view port. */
struct Viewport
{
	int32_t left;
	int32_t top;
	int32_t width;
	int32_t height;
	float nearZ;
	float farZ;

	Viewport()
	:	left(0)
	,	top(0)
	,	width(0)
	,	height(0)
	,	nearZ(0.0f)
	,	farZ(1.0f)
	{
	}

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

/*! \brief Display mode structure. */
struct DisplayMode
{
	uint32_t width;
	uint32_t height;
	uint16_t refreshRate;
	uint16_t colorBits;
	bool stereoscopic;

	DisplayMode()
	:	width(0)
	,	height(0)
	,	refreshRate(0)
	,	colorBits(0)
	,	stereoscopic(false)
	{
	}
};

/*! \brief Render system information. */
struct RenderSystemInformation
{
	uint32_t dedicatedMemoryTotal;
	uint32_t sharedMemoryTotal;
	uint32_t dedicatedMemoryAvailable;
	uint32_t sharedMemoryAvailable;

	RenderSystemInformation()
	:	dedicatedMemoryTotal(0)
	,	sharedMemoryTotal(0)
	,	dedicatedMemoryAvailable(0)
	,	sharedMemoryAvailable(0)
	{
	}
};

/*! \brief Descriptor for render system. */
struct RenderSystemDesc
{
	class IRenderSystem* capture;
	void* nativeHandle;
	int32_t adapter;
	float mipBias;
	int32_t maxAnisotropy;
	bool verbose;

	RenderSystemDesc()
	:	capture(0)
	,	nativeHandle(0)
	,	adapter(-1)
	,	mipBias(0.0f)
	,	maxAnisotropy(1)
	,	verbose(false)
	{
	}
};

/*! \brief Descriptor for render views. */
struct RenderViewDesc
{
	uint16_t depthBits;
	uint16_t stencilBits;
	uint32_t multiSample;
	bool waitVBlank;

	RenderViewDesc()
	:	depthBits(0)
	,	stencilBits(0)
	,	multiSample(0)
	,	waitVBlank(false)
	{
	}
};

/*! \brief Descriptor for default render views. */
struct RenderViewDefaultDesc : public RenderViewDesc
{
	DisplayMode displayMode;
	bool fullscreen;
	std::wstring title;

	RenderViewDefaultDesc()
	:	RenderViewDesc()
	,	fullscreen(true)
	{
	}
};

/*! \brief Descriptor for embedded render views. */
struct RenderViewEmbeddedDesc : public RenderViewDesc
{
	void* nativeWindowHandle;
	bool stereoscopic;

	RenderViewEmbeddedDesc()
	:	RenderViewDesc()
	,	nativeWindowHandle(0)
	,	stereoscopic(false)
	{
	}
};

/*! \brief Immutable texture data. */
struct TextureInitialData
{
	const void* data;
	uint32_t pitch;
	uint32_t slicePitch;

	TextureInitialData()
	:	data(0)
	,	pitch(0)
	,	slicePitch(0)
	{
	}
};

/*! \brief Descriptor for simple textures. */
struct SimpleTextureCreateDesc
{
	int32_t width;
	int32_t height;
	int32_t mipCount;
	TextureFormat format;
	bool sRGB;
	bool immutable;
	TextureInitialData initialData[16];

	SimpleTextureCreateDesc()
	:	width(0)
	,	height(0)
	,	mipCount(0)
	,	format(TfInvalid)
	,	sRGB(false)
	,	immutable(false)
	{
	}
};

/*! \brief Descriptor for cube textures. */
struct CubeTextureCreateDesc
{
	int32_t side;
	int32_t mipCount;
	TextureFormat format;
	bool sRGB;
	bool immutable;
	TextureInitialData initialData[16 * 6];

	CubeTextureCreateDesc()
	:	side(0)
	,	mipCount(0)
	,	format(TfInvalid)
	,	sRGB(false)
	,	immutable(false)
	{
	}
};

/*! \brief Descriptor for volume textures. */
struct VolumeTextureCreateDesc
{
	int32_t width;
	int32_t height;
	int32_t depth;
	int32_t mipCount;
	TextureFormat format;
	bool sRGB;
	bool immutable;
	TextureInitialData initialData[16];

	VolumeTextureCreateDesc()
	:	width(0)
	,	height(0)
	,	depth(0)
	,	mipCount(0)
	,	format(TfInvalid)
	,	sRGB(false)
	,	immutable(false)
	{
	}
};

/*! \brief Descriptor for render target. */
struct RenderTargetCreateDesc
{
	TextureFormat format;	/*< Render target pixel format. */
	bool sRGB;

	RenderTargetCreateDesc()
	:	format(TfInvalid)
	,	sRGB(false)
	{
	}
};

/*! \brief Descriptor for render target sets. */
struct RenderTargetSetCreateDesc
{
	enum { MaxTargets = 8 };

	int32_t count;								/*!< Number of render targets in set; max 4 targets allowed. */
	int32_t width;								/*!< Width of render targets. */
	int32_t height;								/*!< Height of render targets. */
	int32_t multiSample;						/*!< Number of samples; 0 no multisample. */
	bool createDepthStencil;					/*!< Attach depth/stencil buffer; shared among all targets. */
	bool usingDepthStencilAsTexture;			/*!< Will be using depth/stencil buffer as a texture input of shaders. */
	bool usingPrimaryDepthStencil;				/*!< Share primary depth/stencil buffer; shared among all targets. */
	bool preferTiled;							/*!< Prefer tiled memory; only implemented in PS3 renderer. */
	bool ignoreStencil;							/*!< Ignoring stencil; stencil isn't used in rendering. */
	bool generateMips;							/*!< Generate complete mip-chain after target been renderered onto. */
	RenderTargetCreateDesc targets[MaxTargets];	/*!< Descriptor for each target. */

	RenderTargetSetCreateDesc()
	:	count(0)
	,	width(0)
	,	height(0)
	,	multiSample(0)
	,	createDepthStencil(false)
	,	usingDepthStencilAsTexture(false)
	,	usingPrimaryDepthStencil(false)
	,	preferTiled(false)
	,	ignoreStencil(true)
	,	generateMips(false)
	{
	}
};

/*! \brief Draw primitives. */
struct Primitives
{
	PrimitiveType type;
	bool indexed;
	uint32_t offset;
	uint32_t count;
	uint32_t minIndex;
	uint32_t maxIndex;

	Primitives()
	{
	}

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
};

/*! \brief Shader parameter handle. */
typedef uint32_t handle_t;

/*! \brief Return handle from parameter name.
 *
 * \param name Parameter name.
 * \return Parameter handle.
 */
handle_t T_DLLCLASS getParameterHandle(const std::wstring& name);

/*! \brief Synthesize parameter name from index.
 *
 * \param index Texture reference index.
 * \return Parameter name.
 */
std::wstring T_DLLCLASS getParameterNameFromTextureReferenceIndex(int32_t index);

/*! \brief Synthesize parameter handle from index.
 *
 * \param index Texture reference index.
 * \return Parameter handle.
 */
handle_t T_DLLCLASS getParameterHandleFromTextureReferenceIndex(int32_t index);

/*! \brief Return human readable description of data usage. */
std::wstring T_DLLCLASS getDataUsageName(DataUsage usage);

/*! \brief Return human readable description of data type. */
std::wstring T_DLLCLASS getDataTypeName(DataType dataType);

/*! \brief Return number of elements from data type. */
uint32_t T_DLLCLASS getDataElementCount(DataType dataType);

/*! \brief Return human readable description of texture format. */
std::wstring T_DLLCLASS getTextureFormatName(TextureFormat format);

/*! \brief Return byte size from a texture format.
 *
 * \param format Texture format.
 * \return Byte size of block.
 */
uint32_t T_DLLCLASS getTextureBlockSize(TextureFormat format);

/*! \brief Texture block denominator, i.e. block dimension (1x1 or 4x4 etc).
 *
 * \param format Texture format.
 * \return Block denominator.
 */
uint32_t T_DLLCLASS getTextureBlockDenom(TextureFormat format);

/*! \brief Get texture mip size.
 *
 * \param textureSize Size of texture in pixels.
 * \param mipLevel Mip level.
 * \return Mip level size in pixels.
 */
uint32_t T_DLLCLASS getTextureMipSize(uint32_t textureSize, uint32_t mipLevel);

/*! \brief Calculate pitch in bytes from format and width.
 *
 * \param format Texture format.
 * \param textureWidth Width of texture in pixels.
 * \return Texture pitch in bytes.
 */
uint32_t T_DLLCLASS getTextureRowPitch(TextureFormat format, uint32_t textureWidth);

/*! \brief Calculate pitch in bytes from format and width.
*
* \param format Texture format.
* \param textureWidth Width of texture in pixels.
* \param mipLevel Mip level.
* \return Texture pitch in bytes.
*/
uint32_t T_DLLCLASS getTextureRowPitch(TextureFormat format, uint32_t textureWidth, uint32_t mipLevel);

/*! \brief Calculate pitch in bytes from format and width, pitch of an entire mip.
 *
 * \param format Texture format.
 * \param textureWidth Width of texture in pixels.
 * \param textureHeight Height of texture in pixels.
 * \return Mip pitch in bytes.
 */
uint32_t T_DLLCLASS getTextureMipPitch(TextureFormat format, uint32_t textureWidth, uint32_t textureHeight);

/*! \brief Calculate pitch in bytes from format and width, pitch of an entire mip.
 *
 * \param format Texture format.
 * \param textureWidth Width of texture in pixels.
 * \param textureHeight Height of texture in pixels.
 * \param mipLevel Mip level.
 * \return Mip pitch in bytes.
 */
uint32_t T_DLLCLASS getTextureMipPitch(TextureFormat format, uint32_t textureWidth, uint32_t textureHeight, uint32_t mipLevel);

/*! \brief Calculate texture size; assuming continious layout.
 *
 * \param format Texture format.
 * \param textureWidth Width of texture in pixels.
 * \param textureHeight Height of texture in pixels.
 * \param mipLevels Number of mip levels.
 * \return Texture size in bytes.
 */
uint32_t T_DLLCLASS getTextureSize(TextureFormat format, uint32_t textureWidth, uint32_t textureHeight, uint32_t mipLevels);

//@}

	}
}

#endif	// traktor_render_Types_H
