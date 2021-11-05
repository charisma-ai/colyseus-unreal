using System.IO;
using UnrealBuildTool;

namespace UnrealBuildTool.Rules
{
    public class ColyseusClient : ModuleRules
    {
        private string ThirdPartyPath
        {
            get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/")); }
        }

        public ColyseusClient(ReadOnlyTargetRules Target) : base(Target)
        {
            PublicDefinitions.Add("MSGPACK_USE_CPP03=1");

            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            bEnableExceptions = true;

            PublicIncludePaths.AddRange(
                new string[] {
					Path.Combine(ModuleDirectory, "Public"),
                    Path.Combine(ThirdPartyPath, "json/include"),
                    Path.Combine(ThirdPartyPath, "msgpack/include"),
                    // ... add public include paths required here ...
                }
                );


            PrivateIncludePaths.AddRange(
                new string[] {
					Path.Combine(ModuleDirectory, "Private"),
                    // ... add other private include paths required here ...
                }
                );


            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "Json",
                    "JsonUtilities",
                    "WebSockets"
                    // ... add other public dependencies that you statically link with here ...
                }
                );


            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "CoreUObject",
                    "Engine",
                    "Slate",
                    "SlateCore",
                    // ... add private dependencies that you statically link with here ...	
                }
                );


            DynamicallyLoadedModuleNames.AddRange(
                new string[]
                {
                    // ... add any modules that your module loads dynamically here ...
                }
                );
        }
    }
}