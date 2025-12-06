#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <algorithm>

/// <summary>
/// <para> A type-safe event bus for publishing and subscribing to events. </para>
/// <para>:: Marcel Putra 2025 </para>
/// </summary>
class EventBus {
public:
    /// <summary>
    /// Subscribe to a specific event type with a callback
    /// </summary>
    /// <typeparam name="EventType">The type of event to subscribe to.</typeparam>
    /// <param name="callback">A function to call when the event is published.</param>
    /// <returns>An integer subscription ID that can be used to unsubscribe later</returns>
    template<typename EventType>
    int subscribe(std::function<void(const EventType&)> callback) {
        int id = m_nextId++;
        auto wrapper = std::make_shared<CallbackWrapper<EventType>>(id, callback);
        m_callbacks[std::type_index(typeid(EventType))].push_back(wrapper);
        return id;
    }

    /// <summary>
    /// Unsubscribe from a specific event type using a subscription ID.
    /// </summary>
    /// <typeparam name="EventType">The type of event to unsubscribe from</typeparam>
    /// <param name="id">The subscription ID returned from <see cref="subscribe"/>.</param>
    template<typename EventType>
    void unsubscribe(int id) {
        auto it = m_callbacks.find(std::type_index(typeid(EventType)));
        if (it == m_callbacks.end()) return;

        auto& callbacks = it->second;
        callbacks.erase(
            std::remove_if(callbacks.begin(), callbacks.end(),
                [id](const std::shared_ptr<CallbackBase>& cb) {
                    return cb->id == id;
                }),
            callbacks.end()
        );
    }

    /// <summary>
    /// Publish an event to all subscribers of an event type.
    /// </summary>
    /// <typeparam name="EventType">The type of event being published.</typeparam>
    /// <param name="event">The event data to pass to callbacks</param>
    template<typename EventType>
    void publish(const EventType& event) {
        auto it = m_callbacks.find(std::type_index(typeid(EventType)));
        if (it == m_callbacks.end()) return;

        for (auto& callback : it->second) {
            auto wrapper = std::static_pointer_cast<CallbackWrapper<EventType>>(callback);
            wrapper->callback(event);
        }
    }

    /// <summary>
    /// Clears all subscriptions from all event types.
    /// </summary>
    void clear() {
        m_callbacks.clear();
        m_nextId = 0;
    }

    /// <summary>
    /// Clears all subscriptions for a specific event type
    /// </summary>
    /// <typeparam name="EventType">The type of event to clear subscriptions for.</typeparam>
    template<typename EventType>
    void clearType() {
        m_callbacks.erase(std::type_index(typeid(EventType)));
    }

private:
    // base class for storing callback information
    struct CallbackBase {
        int id; // subscription id
        CallbackBase(int id) : id(id) {}
        virtual ~CallbackBase() = default;
    };

    // wrapper for storing a typed callback
    template<typename EventType>
    struct CallbackWrapper : CallbackBase {
        std::function<void(const EventType&)> callback; // actual callback function

        CallbackWrapper(int id, std::function<void(const EventType&)> cb)
            : CallbackBase(id), callback(cb) {
        }
    };

    std::unordered_map<std::type_index, std::vector<std::shared_ptr<CallbackBase>>> m_callbacks;
    int m_nextId = 0; // next subscription id
};