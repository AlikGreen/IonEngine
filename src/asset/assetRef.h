#pragma once

#include <utility>

#include "assetId.h"
#include "core/engine.h"

namespace ion
{
    template <typename T>
    class AssetRef
    {
    public:
        AssetRef() : m_data(nullptr), m_id(0) {  };
        AssetRef(grl::Rc<T> data) : m_data(data), m_id(0) {  }
        AssetRef(T* data) : m_data(data), m_id(0) {  }

        T* operator->() const
        {
            return m_data.get();
        }

        T& operator*() const
        {
            return *m_data;
        }

        T* get() const
        {
            return m_data.get();
        }

        [[nodiscard]] AssetId id() const
        {
            return m_id;
        }

        bool operator==(const AssetRef other) const
        {
            return m_id == other.m_id;
        }

        bool operator!=(const AssetRef other) const
        {
            return m_id != other.m_id;
        }

        operator bool() const
        {
            return m_data != nullptr;
        }
    private:
        friend class AssetRegistry;
        AssetRef(grl::Rc<T> data, const AssetId m_id) : m_data(data), m_id(m_id) {  };

        grl::Rc<T> m_data;
        AssetId    m_id;
    };
}