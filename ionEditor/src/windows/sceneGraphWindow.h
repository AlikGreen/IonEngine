#pragma once
#include <optional>
#include <unordered_set>
#include <entis/entis.h>

#include "editorWindow.h"

namespace ion::Editor
{
class SceneGraphWindow final : public EditorWindow
{
public:
    void update() override;
    void render() override;
private:
    void buildChildrenMap();

    void drawEntityNode(entis::Entity e);

    std::optional<entis::Entity> m_pendingDelete{};
    std::unordered_map<entis::Entity, std::vector<entis::Entity>> m_childrenMap{};
    std::vector<entis::Entity> m_rootEntities{};
    std::unordered_set<size_t> m_expandedEntities{};
    std::string search{};
};
}
