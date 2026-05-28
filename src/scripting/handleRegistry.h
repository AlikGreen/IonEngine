#pragma once
#include <unordered_map>
#include <grl/mem.h>

namespace ion
{
template<typename T>
class HandleRegistry
{
public:
    static HandleRegistry& instance()
    {
        static HandleRegistry s_instance;
        return s_instance;
    }

    uint32_t registerHandle(grl::Rc<T> ptr)
    {
        uint32_t id = m_nextId.fetch_add(1, std::memory_order_relaxed);
        m_handles.emplace(id, std::move(ptr));
        return id;
    }

    T* get(uint32_t id) const
    {
        auto it = m_handles.find(id);
        return it != m_handles.end() ? it->second.get() : nullptr;
    }

    grl::Rc<T> getShared(uint32_t id) const
    {
        auto it = m_handles.find(id);
        return it != m_handles.end() ? it->second : nullptr;
    }

    void release(uint32_t id)
    {
        m_handles.erase(id);
    }

    bool isValid(uint32_t id) const
    {
        return m_handles.contains(id);
    }
private:
    std::unordered_map<uint32_t, grl::Rc<T>> m_handles;
    std::atomic<uint32_t> m_nextId{ 1 };
};
}
