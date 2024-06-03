/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Color4ub.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class StyleBitmap;

/*! Widget style sheet.
 * \ingroup UI
 */
class T_DLLCLASS StyleSheet : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Entity
	{
		std::wstring typeName;
		SmallMap< std::wstring, int32_t > colors;
	};

	//@name Entity
	//@{

	Entity* findEntity(const std::wstring& typeName);

	Entity* addEntity(const std::wstring& typeName);

	//@}

	//@name Palette
	//@{

	int32_t findColor(const Color4ub& color) const;

	int32_t addColor(const Color4ub& color);

	const Color4ub& getColor(int32_t index) const;

	//@}

	//@name Entity element colors.
	//@{

	void setColor(const std::wstring& typeName, const std::wstring_view& element, const Color4ub& color);

	Color4ub getColor(const std::wstring& typeName, const std::wstring_view& element) const;

	Color4ub getColor(const Object* widget, const std::wstring_view& element) const;

	//@}

	//@name Values
	//@{

	void setValue(const std::wstring& name, const std::wstring_view& value);

	std::wstring getValueRaw(const std::wstring_view& name) const;

	std::wstring getValue(const std::wstring_view& name) const;

	//@}

	/*! Merge this style sheet with another.
	 *
	 * Styles defined in right override existing styles.
	 * */
	Ref< StyleSheet > merge(const StyleSheet* right) const;

	virtual void serialize(ISerializer& s) override;

	const AlignedVector< std::wstring >& getInclude() const { return m_include; }

	AlignedVector< Entity >& getEntities() { return m_entities; }

	const AlignedVector< Entity >& getEntities() const { return m_entities; }

	const SmallMap< std::wstring, std::wstring >& getValues() const { return m_values; }

	static Ref< StyleSheet > createDefault();

private:
	AlignedVector< std::wstring > m_include;
	AlignedVector< Entity > m_entities;
	AlignedVector< Color4ub > m_palette;
	SmallMap< std::wstring, std::wstring > m_values;
};

}
