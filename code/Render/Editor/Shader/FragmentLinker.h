/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Guid.h"
#include "Core/Misc/Key.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

#include <functional>
#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ShaderGraph;
class External;

/*! Shader fragment linker.
 * \ingroup Render
 *
 * The fragment linker merges shader graph fragments
 * into a single shader graph by merging named Input- and OutputPorts.
 */
class T_DLLCLASS FragmentLinker : public Object
{
	T_RTTI_CLASS;

public:
	class IFragmentReader
	{
	public:
		virtual ~IFragmentReader() = default;

		/*!
		 */
		virtual Ref< const ShaderGraph > get(const Key& key) const = 0;

		/*!
		 */
		virtual void put(const Key& key, const ShaderGraph* shaderGraph) const = 0;

		/*! Read callback.
		 *
		 * Read fragments from user defined source,
		 * Will automatically be resolved.
		 */
		virtual Ref< const ShaderGraph > read(const Guid& fragmentGuid) const = 0;
	};

	class T_DLLCLASS FragmentReaderNoCache : public IFragmentReader
	{
	public:
		virtual Ref< const ShaderGraph > get(const Key& key) const override final;

		virtual void put(const Key& key, const ShaderGraph* shaderGraph) const override final;
	};

	class T_DLLCLASS FragmentReaderTransientCache : public IFragmentReader
	{
	public:
		explicit FragmentReaderTransientCache(SmallMap< Key, Ref< ShaderGraph > >& cache);

		virtual Ref< const ShaderGraph > get(const Key& key) const override final;

		virtual void put(const Key& key, const ShaderGraph* shaderGraph) const override final;

	private:
		SmallMap< Key, Ref< ShaderGraph > >& m_cache;
	};

	FragmentLinker() = default;

	explicit FragmentLinker(const IFragmentReader& fragmentReader);

	explicit FragmentLinker(const std::function< Ref< const ShaderGraph >(const Guid&) >& fragmentReader);

	virtual ~FragmentLinker();

	/*! Resolve shader graph.
	 *
	 * Resolving External nodes within shader graph and replacing
	 * them with resolved shader graph fragment.
	 *
	 * \param shaderGraph Shader graph to resolve.
	 * \param fullResolve Perform full resolve, child external references as well.
	 * \param optionalShaderGraphGuid GUID of shader graph to be resolved.
	 * \return Resolved shader graph.
	 */
	Ref< ShaderGraph > resolve(const ShaderGraph* shaderGraph, bool fullResolve, const Guid* optionalShaderGraphGuid = nullptr) const;

	/*! Resolve shader graph.
	 *
	 * Resolving External nodes within shader graph and replacing
	 * them with resolved shader graph fragment.
	 *
	 * \param shaderGraph Shader graph to resolve.
	 * \param externalNodes External nodes to resolve, must be owned by shaderGraph.
	 * \param fullResolve Perform full resolve, child external references as well.
	 * \param optionalShaderGraphGuid GUID of shader graph to be resolved.
	 * \return Resolved shader graph.
	 */
	Ref< ShaderGraph > resolve(const ShaderGraph* shaderGraph, const RefArray< External >& externalNodes, bool fullResolve, const Guid* optionalShaderGraphGuid = nullptr) const;

private:
	class LambdaFragmentReader : public FragmentReaderNoCache
	{
	public:
		explicit LambdaFragmentReader(const std::function< Ref< const ShaderGraph >(const Guid&) >& lambda)
			: m_lambda(lambda)
		{
		}

		virtual Ref< const ShaderGraph > read(const Guid& fragmentGuid) const override final
		{
			return m_lambda(fragmentGuid);
		}

	private:
		std::function< Ref< const ShaderGraph >(const Guid&) > m_lambda;
	};

	const IFragmentReader* m_fragmentReader = nullptr;
	bool m_own = false;
};

}
