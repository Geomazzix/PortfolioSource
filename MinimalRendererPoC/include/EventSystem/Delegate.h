#pragma once

#include <signals/signals/signals.hpp>

namespace mrpoc
{
    // Delegate that holds function callbacks.
    // For simplicity, delegates are limited to void return types.
    template<typename... Args>
    class Delegate
    {
    public:
        Delegate() = default;
        ~Delegate() = default;
        using signal = sig::signal<void(Args...)>;
        using connection = typename signal::connection_type;

        template<typename Func>
        connection Connect(Func&& f)
        {
            return m_Callbacks.connect(std::forward<Func>(f));
        }

        template<typename Func, typename Instance>
        connection Connect(Func&& f, Instance&& instance)
        {
            return m_Callbacks.connect(std::forward<Func>(f),instance);
        }

        template<typename Func>
        std::size_t Disconnect(Func&& f)
        {
            return m_Callbacks.disconnect(std::forward<Func>(f));
        }

        template<typename Func, typename Instance>
        std::size_t Disconnect(Func&& f, Instance&& instance)
        {
            return m_Callbacks.disconnect(std::forward<Func>(f), instance);
        }

        void operator()(Args&&... args)
        {
            m_Callbacks(std::forward<Args>(args)...);
        }
    private:
        signal m_Callbacks;
    };
}
