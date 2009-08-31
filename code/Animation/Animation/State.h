#ifndef traktor_animation_State_H
#define traktor_animation_State_H

#include "Core/Serialization/Serializable.h"
#include "Resource/Proxy.h"
#include "Animation/Pose.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class StateContext;
class Animation;

/*! \brief Animation state.
 * \ingroup Animation
 */
class T_DLLCLASS State : public Serializable
{
	T_RTTI_CLASS(State)

public:
	State();

	State(const std::wstring& name, const resource::Proxy< Animation >& animation);

	const std::wstring& getName() const;

	void setPosition(const std::pair< int, int >& position);
	
	const std::pair< int, int >& getPosition() const;

	bool prepareContext(StateContext& outContext);

	void evaluate(
		const StateContext& context,
		Pose& outPose
	);

	virtual bool serialize(Serializer& s);

	inline resource::Proxy< Animation >& getAnimation() { return m_animation; }

	inline const resource::Proxy< Animation >& getAnimation() const { return m_animation; }

private:
	std::wstring m_name;
	std::pair< int, int > m_position;
	resource::Proxy< Animation > m_animation;
};

	}
}

#endif	// traktor_animation_State_H
