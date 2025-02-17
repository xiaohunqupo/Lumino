using System;
using System.Collections.Generic;
using System.IO;

namespace LuminoBuild.Tasks
{
    class BuildEngine_macOS : BuildTask
    {
        public override string CommandName => "BuildEngine_macOS";

        public override void Build(Build builder)
        {
            BuildProject(builder);
        }

        public void BuildProject(Build builder)
        {
            string cmakeOutputDir = Path.Combine(builder.BuildDir, $"macOS", BuildEnvironment.EngineInstallDirName);

            string buildDir = Path.Combine(builder.BuildDir, $"macOS", "EngineBuild");
            Directory.CreateDirectory(buildDir);
            Directory.SetCurrentDirectory(buildDir);

            var args = new string[]
            {
                $"{builder.RootDir}",
                $"-DCMAKE_INSTALL_PREFIX={cmakeOutputDir}",
                $"-DLN_TARGET_ARCH:STRING=macOS",
                $"-DLN_BUILD_TESTS=ON",
                $"-DLN_BUILD_TOOLS=ON",
                $"-DLN_BUILD_EMBEDDED_SHADER_TRANSCOMPILER=ON",
                $"-G", "\"Xcode\"",
            };
            Utils.CallProcess("cmake", string.Join(' ', args));

            Utils.CallProcess("cmake", $"--build . --config Debug");
            Utils.CallProcess("cmake", $"--build . --config Debug --target install");

            Utils.CallProcess("cmake", $"--build . --config Release");
            Utils.CallProcess("cmake", $"--build . --config Release --target install");
        }
    }
}
