/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramResourceVk_H
#define traktor_render_ProgramResourceVk_H

#include "Render/Types.h"
#include "Render/Resource/ProgramResource.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class ProgramResourceVk : public ProgramResource
{
	T_RTTI_CLASS;

public:
	/*! \brief Parameter description. */
	struct ParameterDesc
	{
		std::wstring name;
		uint32_t offset;	//!< Offset in 4-byte floats.
		uint32_t size;		//!< Size in 4-byte floats.

		ParameterDesc()
		:	offset(0)
		,	size(0)
		{
		}

		ParameterDesc(const std::wstring& name_, uint32_t offset_, uint32_t size_)
		:	name(name_)
		,	offset(offset_)
		,	size(size_)
		{
		}

		void serialize(ISerializer& s);
	};

	/*! \brief Mapping from parameter buffer into a uniform buffer. */
	struct ParameterMappingDesc
	{
		uint32_t uniformBufferOffset;	//!< Offset in 4-byte floats.
		uint32_t offset;	//!< Offset in 4-byte floats.
		uint32_t size;		//!< Size in 4-byte floats.

		ParameterMappingDesc()
		:	uniformBufferOffset(0)
		,	offset(0)
		,	size(0)
		{
		}

		ParameterMappingDesc(uint32_t uniformBufferOffset_, uint32_t offset_, uint32_t size_)
		:	uniformBufferOffset(uniformBufferOffset_)
		,	offset(offset_)
		,	size(size_)
		{
		}

		void serialize(ISerializer& s);
	};

	/*! \brief Uniform buffer description and mappings from parameter buffer into uniform buffer. */
	struct UniformBufferDesc
	{
		uint32_t size;	//!< Size in 4-byte floats.
		std::vector< ParameterMappingDesc > parameters;

		UniformBufferDesc()
		:	size(0)
		{
		}

		void serialize(ISerializer& s);
	};

	ProgramResourceVk();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class ProgramVk;
	friend class ProgramCompilerVk;

	RenderState m_renderState;

	std::vector< uint32_t > m_vertexShader;
	std::vector< uint32_t > m_fragmentShader;

	UniformBufferDesc m_vertexUniformBuffers[3];	// Once(0), Frame(1) and Draw(2)
	UniformBufferDesc m_fragmentUniformBuffers[3];

	std::vector< ParameterDesc > m_parameters;
	uint32_t m_parameterScalarSize;
	uint32_t m_parameterTextureSize;
};

	}
}

#endif	// traktor_render_ProgramResourceVk_H
