#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{
	namespace scene
	{

class Scene;

	}

	namespace theater
	{

class Track;

/*! Act
 * \ingroup Theater
 */
class Act : public Object
{
	T_RTTI_CLASS;

public:
	explicit Act(const std::wstring& name, float start, float end, const RefArray< const Track >& tracks);

	bool update(scene::Scene* scene, float time, float deltaTime) const;

	const std::wstring& getName() const { return m_name; }

	float getStart() const { return m_start; }

	float getEnd() const { return m_end; }

private:
	std::wstring m_name;
	float m_start;
	float m_end;
	RefArray< const Track > m_tracks;
};

	}
}

