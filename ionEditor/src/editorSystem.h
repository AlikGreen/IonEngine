#pragma once
#include "core/system.h"
#include "windows/assetsWindow.h"
#include "windows/propertiesWindow.h"
#include "windows/sceneGraphWindow.h"
#include "windows/editorViewportWindow.h"

namespace ion::Editor
{
    struct ComponentInfo
    {
        std::string name;
        std::function<void(entis::Entity entity)> emplaceCallback;
    };

    class EditorSystem final : public System
    {
    public:
        void startup() override;
        void update() override;
        void event(Event *event) override;

        template<typename T>
        void registerComponent(const std::string& name)
        {
            registeredComponents.emplace_back(name, [](entis::Entity entity){ entity.emplace<T>(); });
        }
        std::vector<ComponentInfo> getComponents();
    private:
        void drawMenuBar();
        std::vector<ComponentInfo> registeredComponents{};
        std::vector<grl::Box<EditorWindow>> editorWindows{};
    };
}
