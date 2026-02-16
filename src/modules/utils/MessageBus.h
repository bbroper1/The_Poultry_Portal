#pragma once

#include <Arduino.h>
#include <functional>
#include <map>
#include <queue>

// ---------------------------------------------------------
// MessageBus
// A simple, typed, header-only message bus for inter-module
// communication. Each message type T gets its own isolated
// map of channels -> queues.
// ---------------------------------------------------------
class MessageBus {
public:

    // -----------------------------------------------------
    // Publish a message of type T to a channel
    // -----------------------------------------------------
    template<typename T>
    static void publish(const String& channel, const T& msg) {
        auto& q = queues<T>()[channel];
        q.push(msg);
    }

    // -----------------------------------------------------
    // Poll a channel for messages of type T
    // Calls handler(msg) for each queued message
    // -----------------------------------------------------
    template<typename T>
    static void poll(const String& channel, std::function<void(T&)> handler) {
        auto& q = queues<T>()[channel];
        while (!q.empty()) {
            T msg = q.front();
            q.pop();
            handler(msg);
        }
    }

private:
    // -----------------------------------------------------
    // Each type T gets its own static map of queues
    // -----------------------------------------------------
    template<typename T>
    static std::map<String, std::queue<T>>& queues() {
        static std::map<String, std::queue<T>> _queues;
        return _queues;
    }
};