#include "editorFileSystem.h"

#include <clogr.h>
#include <grl/file.h>

namespace ion::Editor
{
    EditorFileSystem::EditorFileSystem()
    {
        masterRoot = std::make_shared<VFSNode>("Master", "", true, nullptr);
    }

    void EditorFileSystem::mount(const std::string &displayName, const std::string &physicalPath)
    {
        if (!grl::Path::exists(physicalPath))
        {
            clogr::error("Error: Path does not exist: {}", physicalPath);
            return;
        }

        const auto mountNode = std::make_shared<VFSNode>(displayName, physicalPath, true, masterRoot.get());

        scanRecursive(mountNode);
        masterRoot->children.push_back(mountNode);
    }

    void EditorFileSystem::scanRecursive(const grl::Rc<VFSNode> &parentNode)
    {
        for (const auto& entry : fs::directory_iterator(parentNode->physicalPath))
        {

            auto newNode = grl::makeRc<VFSNode>(
                entry.path().filename().string(),
                entry.path(),
                entry.is_directory(),
                parentNode.get()
            );

            if (entry.is_directory())
            {
                scanRecursive(newNode);
            }
            else
            {
                // checkForNestedAssets(newNode);
            }

            parentNode->children.push_back(newNode);
        }
    }
}
