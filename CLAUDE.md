# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

Traktor is a C++20 game engine and editor (MPL-2.0). Windows and Linux are the supported platforms. Android, iOS, macOS and Raspberry Pi projects exist but are not tested. Vulkan is the render backend: a Dx11 backend exists but is disabled in the Win64 solution. Lua is the scripting language.

## Building

There is no CMake. Traktor's own SolutionBuilder generates the projects from `resources/build/{Extern,Traktor}<Platform>.xms`. The bootstrap tools (`Traktor.SolutionBuilder.App`, `Traktor.Run.App`) are checked in prebuilt under `bin/<platform>/releasestatic/`. Generated projects go to `build/<platform>/` and binaries to `bin/latest/<platform>/<config>/`; both are gitignored. Third-party code lives in `3rdp/`, which is also gitignored. It is fetched from the package list in `scripts/misc/packages.run`.

Configurations: `DebugShared`, `ReleaseShared`, `DebugStatic`, `ReleaseStatic`. In the Shared configurations each module is its own DLL/.so, loaded at runtime. CI builds `ReleaseShared`, and the `run-editor` scripts launch it.

Windows (CI uses the VS2026 generator; `build-projects-vs2022-win64.bat` also exists):

```
scripts\update-3rdp.bat                                      :: fetch/update 3rdp; installs a local Vulkan SDK when VULKAN_SDK is unset
scripts\build-projects-vs2026-win64.bat                      :: generate build\win64\Traktor Win64.sln
scripts\autobuild\autobuild-latest-win64.bat ReleaseShared   :: VS env via vswhere + msbuild of the whole solution
scripts\run-editor.bat                                       :: start bin\latest\win64\releaseshared\Traktor.Editor.App.exe
```

In Visual Studio, set `Traktor.Editor.App` as the startup project.

Linux:

```
sudo ./scripts/misc/install-linux-deps.sh
./bin/linux/releasestatic/Traktor.Run.App ./scripts/misc/update-3rdp.run
./scripts/build-projects-make-linux.sh
./scripts/autobuild/autobuild-latest-linux.sh ReleaseShared   # make -f "Extern Linux.mak", then "Traktor Linux.mak"
./scripts/run-editor.sh
```

### Building one module (Windows)

Run this from a VS developer environment (`scripts\config-vs-x64.bat` sets one up). Use PowerShell or cmd, not Git Bash: MSYS path conversion mangles the `/p:` switches.

```
msbuild build\win64\Traktor.Render\Traktor.Render.vcxproj /m /p:Configuration=ReleaseShared /p:Platform=x64 /p:BuildProjectReferences=false "/p:SolutionDir=<repo>\build\win64\\"
```

- `SolutionDir`, with its trailing backslash, is required. Each project's post-build step runs `pushd $(SolutionDir)<Config>` and xcopies the DLL/PDB/LIB into `bin/latest`. Without `SolutionDir`, compilation succeeds but the build fails with MSB3073.
- Add `/t:ClCompile` for a compile-only check with no link.
- Linking fails with LNK1104 while `Traktor.Editor.App` is running, because the editor holds the module DLLs.
- Adding or removing a virtual on an exported interface changes vtables that are compiled into other modules. Rebuild the whole solution in that case. A stale module crashes at runtime.

### Project files, new sources, third-party code

- The `.xms` files glob whole directories (`<fileName>Animation/*.*</fileName>`), so a new file in an existing folder is picked up on the next regeneration. The generated `.vcxproj`/`.vcxproj.filters` list files explicitly. A single-module MSBuild won't see a new file until you regenerate or add the `ClCompile`/`ClInclude` entry by hand.
- A new subfolder, project or dependency must be added to the `.xms` of every platform. The `.xms` files are serialized object graphs with `ref="/object/..."` back-references, so hand edits break easily. `Traktor.SolutionBuilder.Editor.App` (prebuilt in `bin/win64/releasestatic`) edits them.
- A new runtime module must be added to `Editor.Modules` in each `resources/runtime/configurations/Traktor.Editor.<platform>.config`. Otherwise the editor never loads it.
- Generated projects hold absolute paths to the checkout they were generated from (the `code/` include path, `3rdp/`, `bin/latest`). A git worktree has no `build/` or `3rdp/`, and the main checkout's projects compile the main checkout's sources. Building from a worktree therefore needs a copy of the `.vcxproj` with its paths rewritten.
- `3rdp/` is not tracked. Express changes to third-party sources as a `patch = function() ... end` on the package in `scripts/misc/packages.run` (patch files live in `resources/build/patch/`). Direct edits are lost on the next update.

## Tests

Test cases are `traktor::test::Case` subclasses in `code/<Module>/Test/`. They compile into the module itself (there are no separate test projects) and are discovered by RTTI. `scripts/unittest/UnitTest.run` runs every case it finds and prints JUnit XML to stdout:

```
bin\latest\win64\releaseshared\Traktor.Run.App.exe scripts\unittest\UnitTest.run [Traktor.Model Traktor.Render ...]
./bin/latest/linux/releaseshared/Traktor.Run.App ./scripts/unittest/UnitTest.run
```

- By default, only cases from modules that `Traktor.Run.App` already links (Core, Compress) run. Pass module names as arguments (e.g. `Traktor.Model`) to load more.
- There is no per-case filter: the filter block in the script is commented out.
- The script exits 0 even when cases fail, so check `failures=` in the XML.
- A run takes about a minute, mostly `CaseThread`.
- `traktor.test.CaseColor` has a long-standing failure (an exact float compare). It is not a regression.
- CI does not run the tests.

## Architecture

### Modules

`code/<Module>/` is the project `Traktor.<Module>`, in namespace `traktor::<module>`. Includes are rooted at `code/` (`#include "Animation/Skeleton.h"`). Some subfolders are separate projects:
- `<Module>/Editor` becomes `Traktor.<Module>.Editor`. It holds pipelines, editor pages, browse previews and wizards. Editor-only code belongs there.
- Backends: `Render/Vulkan`, `Render/Vrfy`, `Physics/Jolt`, `Physics/Bullet`, `Script/Lua`, `Sound/XAudio2`, `Ui/Win32`, `Ui/X11`, ...
- Executables: `*/App` becomes `Traktor.*.App`.

Exported headers start with the module's export block (`#undef T_DLLCLASS` / `#if defined(T_<MODULE>_EXPORT) ...`) and mark classes `T_DLLCLASS`.

### Core object model (`code/Core`)

- `Object` is intrusively ref-counted. Hold objects with `Ref< T >` / `RefArray< T >`.
- The engine has its own RTTI. C++ RTTI is off in MSVC builds (`/GR-`), so don't use `dynamic_cast` or `typeid`:
  - A class declares `T_RTTI_CLASS;` and implements it with `T_IMPLEMENT_RTTI_CLASS(L"traktor.ns.Name", Class, Super)`.
  - `T_IMPLEMENT_RTTI_FACTORY_CLASS(id, version, Class, Super)` makes a type creatable by name. Anything serialized needs it.
  - `T_IMPLEMENT_RTTI_EDIT_CLASS` also lists the type in the editor's New Instance dialog.
  - Cast with `dynamic_type_cast< T* >`, `checked_type_cast< T* >`, `is_a< T >` and `type_of< T >()`.
- Serialization uses `ISerializable::serialize(ISerializer& s)` with `s >> Member< T >(L"name", m_value)` (also `MemberRef`, `MemberComposite`, `resource::Member`, ...).
  - The same function drives the XML (source DB), binary (output DB), `DeepHash` and `DeepClone` serializers.
  - To change a format, bump the RTTI version and branch on `s.getVersion()` so old data still loads.
  - `MemberComposite` pushes no version scope, so `s.getVersion< T >()` is 0 inside a composited type.
  - The XML serializer keeps only six significant digits for floats.
- There are no registration tables. Pipelines, editor page factories, resource factories, script class factories and test cases are found by scanning RTTI (`TypeInfo::findAllOf`) once their module is loaded.
- In the static configurations, each module's `Module.cpp` must `T_FORCE_LINK_REF` the classes that are only ever created by name. Otherwise the linker strips them.
- Scripting: each module's `*ClassFactory.cpp` exposes C++ classes to Lua through `AutoRuntimeClass< T >` (`addMethod`, `addProperty`, ...).

### Assets: database → pipeline → resources

- **Database (`code/Database`).** Assets are instances identified by GUID.
  - The local provider stores each instance as `<name>.xdm` (GUID and primary type), `<name>.xdi` (the object as XML) and optional binary blobs (`.xdd`).
  - The source database `data/Source` is text and tracked. The output database `data/Output` is binary and gitignored.
  - Raw input files (images, models, sounds) live under `data/Assets`.
  - All three paths are set in `Traktor.workspace`.
- **Pipeline (`code/Editor/IPipeline.h`).** Each asset type has a pipeline.
  - `buildDependencies` declares what an asset uses. `buildOutput` writes the runtime product to the output database.
  - Builds are incremental, keyed on `hashAsset` plus dependency hashes.
  - A type without its own pipeline falls back to `DefaultPipeline` (registered for `Object`), which writes the object to the output database as-is.
  - Headless build: `Traktor.Editor.Build.App <workspace>`, which spawns `Traktor.Pipeline.App`.
- **Resource (`code/Resource`).** At runtime, `IResourceManager::bind(resource::Id< T >, resource::Proxy< T >&)` creates products through the `IResourceFactory` registered for the type. Proxies are what let assets hot-reload (`IResourceManager::reload`).

### Runtime

- `code/World`: entities with components. The serialized data side (`EntityData`, `IEntityComponentData`, `IWorldComponentData`) is turned into runtime objects by `IEntityFactory`. There are three world renderers: `Deferred`, `Forward` and `Simple`.
- `code/Render`:
  - API-neutral interfaces (`IRenderSystem`, `IRenderView`, `IProgram`, ...).
  - The frame graph in `Render/Frame/RenderGraph`, and image-processing graphs in `Render/Image2`.
  - `Render/Vrfy` wraps another render system and validates how the render API is used, so it is the practical spec for API usage rules.
  - Shaders are node graphs (`Render/Editor/Shader`) compiled by the shader pipeline.
  - Vulkan backend sync, upload and cleanup notes: `resources/documentation/vulkan renderer/`.
- `code/Runtime`: the game host `Traktor.Runtime.App`. It has servers for render, audio, physics, input, online and script, plus an `IState` state machine.

### Editor

- `Traktor.Editor.App` loads the modules listed in `Editor.Modules`, then discovers the rest by RTTI: `IEditorPageFactory`/`IEditorPage` (asset editors), `IEditorPlugin`, `IBrowsePreview`, `IWizardTool` and settings pages.
- The UI toolkit is `code/Ui`, the engine's own widget set with Win32, X11, Wayland and Cocoa backends.
- A new asset type typically needs all of these:
  - an EDIT class,
  - a pipeline,
  - an editor page factory (without one, a property dialog opens),
  - an `IBrowsePreview`,
  - an `Editor.BrowseTypeFilter` entry in `resources/runtime/configurations/Traktor.Editor.config`,
  - a caption in `resources/runtime/editor/locale/english/Categories.dictionary` (`TRAKTOR_<NS>_<TYPE>`).
- Other editor UI strings live in `Traktor.<Module>.dictionary` files next to that caption file.
- `code/MCP/Editor` runs an MCP server inside the editor. It speaks JSON-RPC over HTTP on port 13880 (`Editor.McpServerPort`) and exposes the asset database: instances, shader graphs, meshes, textures and more.
- The editor scans the source database at startup. Files written straight into `data/Source` while it runs stay invisible until a restart.

## Conventions

- Formatting follows `code/.clang-format`:
  - tabs,
  - braces on their own lines,
  - spaces inside template brackets (`Ref< T >`),
  - no braces around single-statement bodies,
  - `m_` member prefix.
- Every source file starts with the MPL-2.0 "TRAKTOR" license header. Headers use `#pragma once` and `/*! ... */` Doxygen comments with `\ingroup`.
- Update year range in license header when a file is being modified.
- Strings are mostly `std::wstring` with `L"..."` literals. Log with `log::info << L"..." << Endl;` (also `log::warning`, `log::error`, `log::debug`).
- `.run` files are Lua scripts executed by `Traktor.Run.App`. They drive the 3rdp update, deploy and tests.
- The working tree uses CRLF (`core.autocrlf=true`, no `.gitattributes`). Scripted edits must preserve CRLF.
- Commit subjects are prefixed `Traktor: `.
- Keep comments to one line, if absolutely necessary it can be two lines.
- Comments should be short and condence, never reference bugs or behaviour of code in other places.
- Order of methods in .cpp file should match the order of methods in the class declaration.
- Prefer to extend functionality of system instead of implementing side harnesses.
- Important to keep everything as simple as possible.
- Do not add features which is "good to have" just for the sake of it, preferably only features that has been explicitly requrested or are of immediate use.
- Do not change shared interfaces unless absolutely necessary, try and implement feature in implementation module first.
- Do not use local lambda functions if used only once, implement in place instead.
