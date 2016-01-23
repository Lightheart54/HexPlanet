using UnrealBuildTool;
using System;
using System.IO;

public class PlanetLibrary : ModuleRules
{
    public PlanetLibrary(TargetInfo Target)
    {
        Type = ModuleType.External;

        PublicDependencyModuleNames.AddRange(new string[] { });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        //establish the install directory for your external library
        string libdirectory = System.Environment.GetEnvironmentVariable("PLANET_LIB_ROOT", EnvironmentVariableTarget.User);
        Console.Write(libdirectory + "\n");
        PublicIncludePaths.Add(Path.Combine(libdirectory, "include"));
        libdirectory = Path.Combine(libdirectory, "lib");

        string ConfigurationName;
        switch (Target.Configuration)
        {
            case UnrealTargetConfiguration.Debug:
                ConfigurationName = "Debug";
                break;
            case UnrealTargetConfiguration.DebugGame:
                ConfigurationName = "Debug";
                break;
            default:
                ConfigurationName = "Release";
                break;
        }

        PublicLibraryPaths.Add(libdirectory);
        string libraryName = "PlanetLibrary";
        bool hasDebugSymbolFile = false;
        string debugSymbolFileExt = "";
        string runtimelibExt = "";
        string linktimeLibExt = "";
        string Architecture = "";
        if (Target.Platform == UnrealTargetPlatform.Win32)
        {
            libraryName += "32" + ConfigurationName;
            debugSymbolFileExt = ".pdb";
            runtimelibExt = ".dll";
            linktimeLibExt = ".lib";
            hasDebugSymbolFile = true;
            Architecture = "Win32";
        }
        else if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            libraryName += "64" + ConfigurationName;
            debugSymbolFileExt = ".pdb";
            runtimelibExt = ".dll";
            linktimeLibExt = ".lib";
            hasDebugSymbolFile = true;
            Architecture = "Win64";
        }
        Console.Write(linktimeLibExt + "\n");
        Console.Write(runtimelibExt + "\n");
        Console.Write(debugSymbolFileExt + "\n");

        PublicAdditionalLibraries.Add(libraryName + linktimeLibExt);

        string targetPath = Path.Combine(new string[] { ModuleDirectory, "..","..","Binaries", Architecture });
        if (!System.IO.Directory.Exists(targetPath))
        {
            System.IO.Directory.CreateDirectory(targetPath);
        }
        string sourceFile = Path.Combine(libdirectory, libraryName + runtimelibExt);
        string destFile = Path.Combine(targetPath, libraryName + runtimelibExt);
        RuntimeDependencies.Add(new RuntimeDependency(sourceFile,destFile));
        System.IO.File.Copy(sourceFile, destFile, true);
    }
}