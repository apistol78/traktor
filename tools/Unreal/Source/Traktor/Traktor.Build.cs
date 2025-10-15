using UnrealBuildTool;
using System;
using System.IO;

public class Traktor : ModuleRules
{
	private string[] Libraries =
	{
		"Traktor.Ai",
		"Traktor.Ai.Editor",
		"Traktor.Animation",
		"Traktor.Animation.Editor",
		"Traktor.Avalanche",
		"Traktor.Compress",
		"Traktor.Core",
		"Traktor.Database",
		"Traktor.Database.Compact",
		"Traktor.Database.Local",
		"Traktor.Database.Remote",
		"Traktor.Database.Remote.Client",
		"Traktor.Database.Remote.Server",
		"Traktor.Drawing",
		"Traktor.Editor",
		"Traktor.Heightfield",
		"Traktor.Heightfield.Editor",
		"Traktor.Html",
		"Traktor.I18N",
		"Traktor.I18N.Editor",
		"Traktor.Input",
		"Traktor.Input.Editor",
		"Traktor.Json",
		"Traktor.Jungle",
		"Traktor.Mesh",
		"Traktor.Mesh.Editor",
		"Traktor.Model",
		"Traktor.Model.Editor",
		"Traktor.Net",
		"Traktor.Online",
		"Traktor.Physics",
		"Traktor.Physics.Editor",
		"Traktor.Render",
		"Traktor.Render.Vrfy",
		"Traktor.Render.Editor",
		"Traktor.Resource",
		"Traktor.Resource.Editor",
		"Traktor.Runtime",
		"Traktor.Runtime.Editor",
		"Traktor.Scene",
		"Traktor.Scene.Editor",
		"Traktor.Script",
		"Traktor.Script.Editor",
		"Traktor.Shape.Editor",
		"Traktor.Sound",
		"Traktor.Sound.Editor",
		"Traktor.Spark",
		"Traktor.Spark.Editor",
		"Traktor.Spray",
		"Traktor.Spray.Editor",
		"Traktor.Sql",
		"Traktor.Sql.Sqlite3",
		"Traktor.Svg",
		"Traktor.Terrain",
		"Traktor.Terrain.Editor",
		"Traktor.Theater",
		"Traktor.Theater.Editor",
		"Traktor.Ui",
		"Traktor.Video",
		"Traktor.Video.Editor",
		"Traktor.Weather",
		"Traktor.Weather.Editor",
		"Traktor.World",
		"Traktor.World.Editor",
		"Traktor.Xml",
	};
	
    public Traktor(ReadOnlyTargetRules Target) : base(Target)
    {
        //Type = ModuleType.External;

		string TraktorHome = Environment.GetEnvironmentVariable("TRAKTOR_HOME");
        string BasePath = TraktorHome; // Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "traktor");

        PublicSystemIncludePaths.Add(Path.Combine(BasePath, "code"));

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
			foreach (string library in Libraries)
			{
				PublicAdditionalLibraries.Add(Path.Combine(BasePath, "bin/latest/win64/releaseshared", library + ".lib"));
				//PublicDelayLoadDLLs.Add(library + ".dll"); // only if dynamic
				RuntimeDependencies.Add("$(BinaryOutputDir)/" + library + ".dll",
										Path.Combine(BasePath, "bin/latest/win64/releaseshared", library + ".dll"));
			}
        }
    }
}
