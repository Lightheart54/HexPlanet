using UnrealBuildTool;
using System;
using System.IO;

public class boost : ModuleRules
{
    public boost(TargetInfo Target)
    {
        Type = ModuleType.External;

        PublicDependencyModuleNames.AddRange(new string[] { });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        //establish the install directory for your external library
        string libdirectory = System.Environment.GetEnvironmentVariable("BOOST_ROOT", EnvironmentVariableTarget.User) + "/";
        Console.Write(libdirectory + "\n");
        PublicIncludePaths.Add(libdirectory);
    }
}