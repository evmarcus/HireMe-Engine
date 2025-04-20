#pragma once
#ifndef EVENTBUS_H
#define EVENTBUS_H

#include "utility.h"
#include <unordered_map>

class Subscription {
public:
    std::string event_type;
    std::shared_ptr<luabridge::LuaRef> component;
    std::shared_ptr<luabridge::LuaRef> function;

    Subscription(std::string _event_type, const luabridge::LuaRef& _component, const luabridge::LuaRef& _function);

    void Callback(const luabridge::LuaRef& event_object);

    ~Subscription();
};

class EventBus
{
public:
    static inline std::unordered_map<std::string, std::vector<Subscription*>> events;

    //static inline std::unordered_map<Subscription*> already_subscribed;

    // Line of new subscribers (true) and unsubscribers (false)
    static inline std::vector<std::pair<bool, Subscription*>> subscription_tasks;

    //static inline std::vector<Subscription*> new_subscribers;

    //static inline std::vector<Subscription*> unsubscribers;

    static void ProcessSubscriptions();

    static void Publish(std::string event_type);

    static void Publish(std::string event_type, const luabridge::LuaRef& event_object);

    static void Subscribe(std::string event_type, const luabridge::LuaRef& component, const luabridge::LuaRef& function);

    static void Unsubscribe(std::string event_type, const luabridge::LuaRef& component, const luabridge::LuaRef& function);
};

#endif