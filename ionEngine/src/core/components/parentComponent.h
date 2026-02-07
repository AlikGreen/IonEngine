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
    private:
        entis::Entity parent = entis::Entity::null();
    };
}