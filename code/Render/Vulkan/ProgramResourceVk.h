#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"
#include "Render/Resource/ProgramResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_VULKAN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class T_DLLCLASS ProgramResourceVk : public ProgramResource
{
	T_RTTI_CLASS;

public:
	struct ParameterDesc
	{
		std::wstring name;
		uint32_t buffer;	//!< Index into which uniform buffer (0-2).
		uint32_t offset;	//!< Offset in 4-byte floats.
		uint32_t size;		//!< Size in 4-byte floats.

		ParameterDesc()
		:	buffer(0)
		,	offset(0)
		,	size(0)
		{
		}

		ParameterDesc(const std::wstring& name_, uint32_t buffer_, uint32_t offset_, uint32_t size_)
		:	name(name_)
		,	buffer(buffer_)
		,	offset(offset_)
		,	size(size_)
		{
		}

		void serialize(ISerializer& s);
	};

	struct SamplerDesc
	{
		uint32_t binding;
		SamplerState state;

		SamplerDesc()
		:	binding(0)
		{
		}

		SamplerDesc(uint32_t binding_, const SamplerState& state_)
		:	binding(binding_)
		,	state(state_)
		{
		}

		void serialize(ISerializer& s);
	};

	struct TextureDesc
	{
		uint32_t binding;

		TextureDesc()
		:	binding(0)
		{
		}

		explicit TextureDesc(uint32_t binding_)
		:	binding(binding_)
		{
		}

		void serialize(ISerializer& s);
	};

	struct SBufferDesc
	{
		uint32_t binding;

		SBufferDesc()
		:	binding(0)
		{
		}

		explicit SBufferDesc(uint32_t binding_)
		:	binding(binding_)
		{
		}

		void serialize(ISerializer& s);
	};

	ProgramResourceVk();

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ProgramVk;
	friend class ProgramCompilerVk;

	RenderState m_renderState;

	AlignedVector< uint32_t > m_vertexShader;
	AlignedVector< uint32_t > m_fragmentShader;

	AlignedVector< ParameterDesc > m_parameters;
	uint32_t m_uniformBufferSizes[3];	// Once(0), Frame(1) and Draw(2)

	AlignedVector< SamplerDesc > m_samplers;
	AlignedVector< TextureDesc > m_textures;
	AlignedVector< SBufferDesc > m_sbuffers;

	uint32_t m_hash;
};

	}
}

