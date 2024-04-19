/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/Engine/Layer.h"
#include "Core/Class/Any.h"
#include "Core/Math/Vector2.h"
#include "Resource/Id.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::runtime
{

class IEnvironment;

}

namespace traktor::render
{

class IRenderTargetSet;

}

namespace traktor::spark
{

class AccDisplayRenderer;
class Context;
class Movie;
class MoviePlayer;
class MovieRenderer;
class SpriteInstance;
class ISoundRenderer;

/*! Stage Spark layer.
 * \ingroup Spark
 */
class T_DLLCLASS SparkLayer : public runtime::Layer
{
	T_RTTI_CLASS;

public:
	explicit SparkLayer(
		runtime::Stage* stage,
		const std::wstring& name,
		bool permitTransition,
		runtime::IEnvironment* environment,
		const resource::Proxy< Movie >& movie,
		const std::map< std::wstring, resource::Proxy< Movie > >& externalMovies,
		bool clearBackground,
		bool enableSound
	);

	virtual void destroy() override;

	virtual void transition(Layer* fromLayer) override final;

	virtual void preUpdate(const runtime::UpdateInfo& info) override final;

	virtual void update(const runtime::UpdateInfo& info) override final;

	virtual void postUpdate(const runtime::UpdateInfo& info) override final;

	virtual void preSetup(const runtime::UpdateInfo& info) override final;

	virtual void setup(const runtime::UpdateInfo& info, render::RenderGraph& renderGraph) override final;

	virtual void preReconfigured() override final;

	virtual void postReconfigured() override final;

	virtual void suspend() override final;

	virtual void resume() override final;

	virtual void hotReload() override final;

	/*! Get reference to current Spark movie player.
	 *
	 * \return Spark movie player object.
	 */
	MoviePlayer* getMoviePlayer();

	/*! Get context.
	 *
	 * \return Context.
	 */
	Context* getContext();

	/*! Get "_root" sprite.
	 *
	 * \return "_root" sprite.
	 */
	SpriteInstance* getRoot();

	/*! Get external movie.
	 *
	 * \param id Name of external movie.
	 * \return External movie, null if doesn't exist.
	 */
	Movie* getExternal(const std::wstring& id) const;

	/*! Get "safe" string which only contain glyphs which are valid with loaded fonts.
	 *
	 * \param text Input text with potential unprintable glyphs.
	 * \param empty String to return if no glyphs in input text was valid.
	 * \return String containing only printable glyphs.
	 */
	std::wstring getPrintableString(const std::wstring& text, const std::wstring& empty) const;

	/*! Set if Spark should be rendererd.
	 *
	 * \param visible True if Spark should be rendered.
	 */
	void setVisible(bool visible) { m_visible = visible; }

	/*! Check if Spark is being rendered.
	 *
	 * \return True if Spark is being rendered.
	 */
	bool isVisible() const { return m_visible; }

private:
	struct LastMouseState
	{
		int32_t button = 0;
		int32_t wheel = 0;
	};

	Ref< runtime::IEnvironment > m_environment;
	resource::Proxy< Movie > m_movie;
	std::map< std::wstring, resource::Proxy< Movie > > m_externalMovies;
	Ref< MoviePlayer > m_moviePlayer;
	Ref< MovieRenderer > m_movieRenderer;
	Ref< AccDisplayRenderer > m_displayRenderer;
	Ref< ISoundRenderer > m_soundRenderer;
	bool m_clearBackground;
	bool m_enableSound;
	bool m_visible;
	Vector2 m_offset;
	float m_scale;
	bool m_lastUpValue;
	bool m_lastDownValue;
	bool m_lastConfirmValue;
	bool m_lastEscapeValue;
	LastMouseState m_lastMouse[8];
	int32_t m_lastMouseX;
	int32_t m_lastMouseY;

	void createMoviePlayer();
};

}
