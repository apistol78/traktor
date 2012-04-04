#ifndef traktor_parade_FlashLayer_H
#define traktor_parade_FlashLayer_H

#include "Resource/Proxy.h"
#include "Parade/Layer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;

	}

	namespace flash
	{

class AccDisplayRenderer;
class ActionObject;
class FlashMovie;
class FlashMoviePlayer;
class ISoundRenderer;

	}

	namespace parade
	{

class T_DLLCLASS FlashLayer : public Layer
{
	T_RTTI_CLASS;

public:
	FlashLayer(
		const std::wstring& name,
		amalgam::IEnvironment* environment,
		const resource::Proxy< script::IScriptContext >& scriptContext,
		const resource::Proxy< flash::FlashMovie >& movie,
		bool clearBackground
	);

	virtual void update(Stage* stage, const amalgam::IUpdateInfo& info);

	virtual void build(Stage* stage, const amalgam::IUpdateInfo& info, uint32_t frame);

	virtual void render(Stage* stage, render::EyeType eye, uint32_t frame);

	virtual void leave(Stage* stage);

	virtual void reconfigured(Stage* stage);

	flash::FlashMoviePlayer* getMoviePlayer();

	flash::ActionObject* getGlobal();

	flash::ActionObject* getRoot();

private:
	Ref< amalgam::IEnvironment > m_environment;
	resource::Proxy< flash::FlashMovie > m_movie;
	Ref< flash::FlashMoviePlayer > m_moviePlayer;
	Ref< flash::AccDisplayRenderer > m_displayRenderer;
	Ref< flash::ISoundRenderer > m_soundRenderer;
	bool m_clearBackground;

	void createMoviePlayer();
};

	}
}

#endif	// traktor_parade_FlashLayer_H
