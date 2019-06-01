#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
#include "Illuminate/Editor/Types.h"
#include "Model/Model.h"

namespace traktor
{

class Transform;

	namespace illuminate
	{

class GBuffer : public Object
{
	T_RTTI_CLASS;

public:
	struct Element
	{
		uint32_t polygon;
		uint32_t material;
		Vector4 position;
		Vector4 normal;
		Vector4 delta;
		bool inside;
		
		Element()
		:	polygon(model::c_InvalidIndex)
		,	material(model::c_InvalidIndex)
		,	position(Vector4::origo())
		,	normal(Vector4::zero())
		,	delta(Vector4::zero())
		,	inside(false)
		{
		}
	};

	GBuffer();

	bool create(int32_t width, int32_t height, const model::Model& model, const Transform& transform, uint32_t texCoordChannel);

	void set(int32_t x, int32_t y, const Element& elm) { m_data[x + y * m_width] = elm; }

	const Element& get(int32_t x, int32_t y) const { return m_data[x + y * m_width]; }

	Element& get(int32_t x, int32_t y) { return m_data[x + y * m_width]; }

	int32_t getWidth() const { return m_width; }

	int32_t getHeight() const { return m_height; }

	const Aabb3& getBoundingBox() const { return m_boundingBox; }

	void saveAsImages(const std::wstring& outputPath) const;

private:
	int32_t m_width;
	int32_t m_height;
	AlignedVector< Element > m_data;
	Aabb3 m_boundingBox;
};

	}
}

