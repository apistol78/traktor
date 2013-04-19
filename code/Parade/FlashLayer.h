#ifndef traktor_parade_FlashLayer_H
#define traktor_parade_FlashLayer_H

#include "Flash/Action/Avm1/Classes/As_flash_external_ExternalInterface.h"
#include "Parade/Layer.h"
#include "Resource/Proxy.h"
#include "Script/Any.h"

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

	namespace drawing
	{

class Image;

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

class T_DLLCLASS FlashLayer
:	public Layer
,	public flash::IExternalCall
{
	T_RTTI_CLASS;

public:
	FlashLayer(
		Stage* stage,
		const std::wstring& name,
		amalgam::IEnvironment* environment,
		const resource::Proxy< flash::FlashMovie >& movie,
		bool clearBackground,
		bool enableSound
	);

	virtual ~FlashLayer();

	void destroy();

	virtual void prepare();

	virtual void update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info);

	virtual void build(const amalgam::IUpdateInfo& info, uint32_t frame);

	virtual void render(render::EyeType eye, uint32_t frame);

	virtual void reconfigured();

	flash::FlashMoviePlayer* getMoviePlayer();

	flash::ActionObject* getGlobal();

	flash::ActionObject* getRoot();

	Ref< flash::ActionObject > createObject() const;

	Ref< flash::ActionObject > createObject(uint32_t argc, const script::Any* argv) const;

	Ref< flash::ActionObject > createBitmap(drawing::Image* image) const;

	script::Any externalCall(const std::string& methodName, uint32_t argc, const script::Any* argv);

	void setVisible(bool visible) { m_visible = visible; }

	bool isVisible() const { return m_visible; }

private:
	Ref< amalgam::IEnvironment > m_environment;
	resource::Proxy< flash::FlashMovie > m_movie;
	Ref< flash::FlashMoviePlayer > m_moviePlayer;
	Ref< flash::AccDisplayRenderer > m_displayRenderer;
	Ref< flash::ISoundRenderer > m_soundRenderer;
	bool m_clearBackground;
	bool m_enableSound;
	bool m_visible;
	int32_t m_lastX;
	int32_t m_lastY;
	int32_t m_lastButton;
	int32_t m_lastWheel;

	void createMoviePlayer();

	virtual flash::ActionValue dispatchExternalCall(const std::string& methodName, int32_t argc, const flash::ActionValue* argv);
};

	}
}

#endif	// traktor_parade_FlashLayer_H
