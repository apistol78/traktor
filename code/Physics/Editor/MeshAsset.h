/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Core/Guid.h"
#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

class T_DLLCLASS MeshAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	void setImportFilter(const std::wstring& importFilter) { m_importFilter = importFilter; }

	const std::wstring& getImportFilter() const { return m_importFilter; }

	void setCalculateConvexHull(bool calculateConvexHull) { m_calculateConvexHull = calculateConvexHull; }

	bool getCalculateConvexHull() const { return m_calculateConvexHull; }

	void setMargin(float margin) { m_margin = margin; }

	float getMargin() const { return m_margin; }

	void setMaterials(const std::map< std::wstring, Guid >& materials) { m_materials = materials; }

	const std::map< std::wstring, Guid >& getMaterials() const { return m_materials; }

private:
	friend class MeshPipeline;

	std::wstring m_importFilter;
	bool m_calculateConvexHull = true;
	float m_margin = 0.04f;
	std::map< std::wstring, Guid > m_materials;	//!< References to Material instances.
};

}
