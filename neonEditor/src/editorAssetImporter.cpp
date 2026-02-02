#include "editorAssetImporter.h"

#include "asset/assetManager.h"
#include "asset/assetRef.h"
#include "core/engine.h"
#include "core/scene.h"

namespace Neon
{
    AssetID EditorAssetImporter::import(const std::string &sourcePath, const std::string &destFolder)
    {
        std::string filename = Path::stem(sourcePath) + ".asset";
        std::string outputFolder = Path::join(Engine::getConfig().resourcePaths.at(0), destFolder);
        std::string outputPath = outputFolder + "/" + filename;

        if (Path::exists(outputPath))
        {
            int counter = 1;
            while (Path::exists(Path::join(outputFolder, Path::stem(sourcePath) + "_" + std::to_string(counter) + ".asset")))
            {
                counter++;
            }
            filename = Path::stem(sourcePath) + "_" + std::to_string(counter) + ".asset";
            outputPath = outputFolder + "/" + filename;
        }

        const std::string ext = Path::extension(sourcePath);

        if (ext == ".glb")
        {
            const AssetRef<Scene> scene = Engine::getAssetManager().import<Scene>(sourcePath);
            saveAsAssetFile(scene, outputPath);

            Log::info("Imported and saved at {}", outputPath);
            return scene.id();
        }

        return AssetID(0);
    }
}
