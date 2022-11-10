/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Containers/SmallMap.h"
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

/*! Shader resource base class.
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
		float value = 0.0f;

		InitializeUniformScalar() = default;

		explicit InitializeUniformScalar(const std::wstring& name_, float value_)
		:	name(name_)
		,	value(value_)
		{
		}

		void serialize(ISerializer& s);
	};

	struct InitializeUniformVector
	{
		std::wstring name;
		Vector4 value = Vector4::zero();

		InitializeUniformVector() = default;

		explicit InitializeUniformVector(const std::wstring& name_, const Vector4& value_)
		:	name(name_)
		,	value(value_)
		{
		}

		void serialize(ISerializer& s);
	};

	struct Combination
	{
		uint32_t mask = 0;
		uint32_t value = 0;
		uint32_t priority = 0;
		Ref< ISerializable > program;
		AlignedVector< Guid > textures;
		AlignedVector< InitializeUniformScalar > initializeUniformScalar;
		AlignedVector< InitializeUniformVector > initializeUniformVector;

		void serialize(ISerializer& s);
	};

	struct Technique
	{
		std::wstring name;
		uint32_t mask = 0;
		AlignedVector< Combination > combinations;

		void serialize(ISerializer& s);
	};

	const SmallMap< std::wstring, uint32_t >& getParameterBits() const;

	const AlignedVector< Technique >& getTechniques() const;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ShaderPipeline;

	SmallMap< std::wstring, uint32_t > m_parameterBits;
	AlignedVector< Technique > m_techniques;
};

	}
}

