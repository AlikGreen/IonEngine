#include "editorAssetImporter.h"

#include "asset/assetManager.h"
#include "asset/assetRef.h"
#include "core/engine.h"
#include "core/scene.h"

namespace ion::Editor
{
    AssetId EditorAssetImporter::import(const std::string &sourcePath, const std::string &destFolder)
    {
        std::string filename = grl::Path::stem(sourcePath) + ".asset";
        std::string outputFolder = grl::Path::join(Engine::getConfig().resourcePaths.at(0), destFolder);
        std::string outputPath = outputFolder + "/" + filename;

        if (grl::Path::exists(outputPath))
        {
            int counter = 1;
            while (grl::Path::exists(grl::Path::join(outputFolder, grl::Path::stem(sourcePath) + "_" + std::to_string(counter) + ".asset")))
            {
                counter++;
            }
            filename = grl::Path::stem(sourcePath) + "_" + std::to_string(counter) + ".asset";
            outputPath = outputFolder + "/" + filename;
        }

        const std::string ext = grl::Path::extension(sourcePath);

        if (ext == ".glb")
        {
            const AssetRef<Scene> scene = Engine::getAssetManager().import<Scene>(sourcePath);
            saveAsAssetFile(scene, outputPath);

            clogr::info("Imported and saved at {}", outputPath);
            return scene.id();
        }

        return AssetId::invalid();
    }
}
