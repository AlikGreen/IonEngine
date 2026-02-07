#pragma once
#include <concepts>

namespace ion
{
    class Event
    {
    public:
        virtual ~Event() = default;

        void cancel()
        {
            canceled = true;
        }

        [[nodiscard]] bool isCanceled() const
        {
            return canceled;
        }

        template<typename... Types>
        [[nodiscard]] bool isAny() const
        {
            return ( (dynamic_cast<const Types*>(this) != nullptr) || ... );
        }
    private:
        bool canceled = false;
    };

    template <typename T>
    concept DerivedFromEvent = std::derived_from<T, Event>;
}
