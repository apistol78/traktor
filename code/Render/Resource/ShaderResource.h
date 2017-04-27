/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ShaderResource_H
#define traktor_render_ShaderResource_H

#include <map>
#include <vector>
#include "Core/Guid.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"

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

/*! \brief Shader resource base class.
 * \ingroup Render
 *
 * A shader resource is an intermediate representation
 * of a generated and compiled shader.
 */
class T_DLLCLASS ShaderResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct InitializeUniformScalar
	{
		std::wstring name;
		float value;

		InitializeUniformScalar()
		:	value(0.0f)
		{
		}

		InitializeUniformScalar(const std::wstring& name_, float value_)
		:	name(name_)
		,	value(value_)
		{
		}

		void serialize(ISerializer& s);
	};

	struct InitializeUniformVector
	{
		std::wstring name;
		Vector4 value;

		InitializeUniformVector()
		:	value(Vector4::zero())
		{
		}

		InitializeUniformVector(const std::wstring& name_, const Vector4& value_)
		:	name(name_)
		,	value(value_)
		{
		}

		void serialize(ISerializer& s);
	};

	struct Combination
	{
		uint32_t mask;
		uint32_t value;
		uint32_t priority;
		Ref< ISerializable > program;
		std::vector< Guid > textures;
		AlignedVector< InitializeUniformScalar > initializeUniformScalar;
		AlignedVector< InitializeUniformVector > initializeUniformVector;

		Combination()
		:	mask(0)
		,	value(0)
		,	priority(0)
		{
		}

		void serialize(ISerializer& s);
	};

	struct Technique
	{
		std::wstring name;
		uint32_t mask;
		std::vector< Combination > combinations;

		Technique()
		:	mask(0)
		{
		}

		void serialize(ISerializer& s);
	};

	const std::map< std::wstring, uint32_t >& getParameterBits() const;

	const std::vector< Technique >& getTechniques() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class ShaderPipeline;

	std::map< std::wstring, uint32_t > m_parameterBits;
	std::vector< Technique > m_techniques;
};

	}
}

#endif	// traktor_render_ShaderResource_H
