#include "EventBus.h"
#include "ComponentManager.h"

Subscription::Subscription(std::string _event_type, const luabridge::LuaRef& _component, const luabridge::LuaRef& _function) {
    event_type = _event_type;
    component = std::make_shared<luabridge::LuaRef>(_component);
    function = std::make_shared<luabridge::LuaRef>(_function);
}

void Subscription::Callback(const luabridge::LuaRef& event_object) {
    // If event is not passed in, it's nil
    if (event_object == luabridge::LuaRef(ComponentManager::GetLuaState())) {
        try {
            (*function)(*component);
        }
        catch (const luabridge::LuaException& e) {
            EngineUtils::ReportError((*component)["name"], e);
        }
    }
    else {
        try {
            (*function)(*component, event_object);
        }
        catch (const luabridge::LuaException& e) {
            EngineUtils::ReportError((*component)["name"], e);
        }
    }
}

Subscription::~Subscription()
{
    EventBus::Unsubscribe(event_type, *component, *function);
}

void EventBus::ProcessSubscriptions() {
    for (std::pair<bool, Subscription*>& subscription_task : subscription_tasks) {
        Subscription* subscription = subscription_task.second;
        // if first is true, it was a new subscription
        if (subscription_task.first) {
            events[subscription->event_type].emplace_back(subscription);
        }
        // if first is false, it was an unsubscription
        else {
            auto subscriber_list = events.find(subscription->event_type);

            // TODO OPT: Find a way around doing this search twice (once in unsubscribe and once here)
            if (subscriber_list != events.end() && !subscriber_list->second.empty()) {
                // Remove the subscription from the vector at events[subscription->event_type] if it is equal in every way, even if
                auto it = std::find(subscriber_list->second.begin(), subscriber_list->second.end(), subscription);
                if (it != subscriber_list->second.end()) {
                    subscriber_list->second.erase(it); // Erase only the found element
                }
            }
        }
    }

    subscription_tasks.clear();
}

void EventBus::Publish(std::string event_type) {
    Publish(event_type, luabridge::LuaRef(ComponentManager::GetLuaState()));
}

void EventBus::Publish(std::string event_type, const luabridge::LuaRef& event_object) {

    auto subscriber_list = events.find(event_type);

    if (subscriber_list != events.end() && !subscriber_list->second.empty()) {
        
        std::vector<Subscription*> orphaned_subscriptions;

        for (Subscription* subscription : subscriber_list->second) {
            if ((*subscription->component)["enabled"].cast<bool>() == false) {
                orphaned_subscriptions.emplace_back(subscription);
            }
            else {
                subscription->Callback(event_object);
            }
        }

        if (!orphaned_subscriptions.empty()) {
            for (Subscription* orphaned_subscription : orphaned_subscriptions) {
                Unsubscribe(orphaned_subscription->event_type, *orphaned_subscription->component, *orphaned_subscription->function);
            }
        }
    }
}

void EventBus::Subscribe(std::string event_type, const luabridge::LuaRef& component, const luabridge::LuaRef& function) {
    Subscription* subscription = new Subscription(event_type, component, function);

    subscription_tasks.emplace_back(true, subscription);
}

void EventBus::Unsubscribe(std::string event_type, const luabridge::LuaRef& component, const luabridge::LuaRef& function) {
    auto subscriber_list = events.find(event_type);

    if (subscriber_list != events.end() && !subscriber_list->second.empty()) {
        auto it = std::find_if(subscriber_list->second.begin(), subscriber_list->second.end(), [&](Subscription* sub) {
            return sub && sub->component && sub->function &&
                *sub->component == component &&
                *sub->function == function;
        });

        if (it != subscriber_list->second.end()) {
            Subscription* foundSubscription = *it;
            subscription_tasks.emplace_back(false, foundSubscription);
        }
        else {
            /*auto it2 = std::find_if(subscription_tasks.begin(), subscription_tasks.end(), [&](const std::pair<bool, Subscription*>& pair) {
                Subscription* sub = pair.second;
                return pair.first && sub && sub->component && sub->function &&
                    *sub->component == component &&
                    *sub->function == function;
            });

            if (it2 != subscription_tasks.end()) {
                subscription_tasks.erase(it2); // Remove the found element
            }*/

            subscription_tasks.erase(std::remove_if(subscription_tasks.begin(), subscription_tasks.end(),
                [&](const std::pair<bool, Subscription*>& pair) {
                    Subscription* sub = pair.second;
                    return pair.first && sub && sub->component && sub->function &&
                        *sub->component == component &&
                        *sub->function == function;
                }),
                subscription_tasks.end());
        }
    }
}
