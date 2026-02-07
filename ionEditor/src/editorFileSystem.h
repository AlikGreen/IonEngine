#pragma once
#pragma once

#include <string>
#include <utility>
#include <vector>
#include <filesystem>

#include "asset/assetId.h"

namespace fs = std::filesystem;

namespace ion::Editor
{
struct VFSNode
{
    std::string name;
    fs::path physicalPath;
    bool isDirectory;

    std::vector<grl::Rc<VFSNode>> children;
    AssetId asset;

    VFSNode* parent = nullptr;

    VFSNode(std::string n, fs::path p, const bool isDir, VFSNode* par)
        : name(std::move(n)), physicalPath(std::move(p)), isDirectory(isDir), parent(par) {}
};

class EditorFileSystem
{
public:
    EditorFileSystem();

    void mount(const std::string& displayName, const std::string& physicalPath);
    grl::Rc<VFSNode> getRoot() { return masterRoot; }
private:
    void scanRecursive(const grl::Rc<VFSNode> &parentNode);
    grl::Rc<VFSNode> masterRoot;
};
}