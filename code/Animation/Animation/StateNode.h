#ifndef traktor_animation_StateNode_H
#define traktor_animation_StateNode_H

#include "Animation/Pose.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace animation
	{

class StateContext;

/*! \brief Animation state node.
 * \ingroup Animation
 */
class T_DLLCLASS StateNode : public ISerializable
{
	T_RTTI_CLASS;

public:
	StateNode();

	StateNode(const std::wstring& name);

	const std::wstring& getName() const;

	void setPosition(const std::pair< int, int >& position);
	
	const std::pair< int, int >& getPosition() const;

	virtual bool bind(resource::IResourceManager* resourceManager) = 0;

	virtual bool prepareContext(StateContext& outContext) = 0;

	virtual void evaluate(StateContext& context, Pose& outPose) = 0;

	virtual void serialize(ISerializer& s) T_OVERRIDE;

private:
	std::wstring m_name;
	std::pair< int, int > m_position;
};

	}
}

#endif	// traktor_animation_StateNode_H
