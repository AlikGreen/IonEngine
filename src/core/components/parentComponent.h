#pragma once
#include <entis/entis.h>

namespace ion
{
    struct Parent
    {
    public:
        [[nodiscard]] bool hasParent() const
        {
            return parent.isValid();
        }

        [[nodiscard]] entis::Entity getParent() const
        {
            return parent;
        }

        void setParent(const entis::Entity parent)
        {
            this->parent = parent;
        }

        void remapEntities(const std::unordered_map<entis::EntityId, entis::Entity>& idMap)
        {
            const auto it = idMap.find(parent.id());
            if (it != idMap.end())
                parent = it->second;
            else
                parent = entis::Entity::null();
        }
    private:
        entis::Entity parent = entis::Entity::null();
    };
}