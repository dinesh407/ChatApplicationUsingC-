#ifndef NETWORK_FUNCTION_HPP
#define NETWORK_FUNCTION_HPP

#include "Types.hpp"
#include "Logger.hpp"
#include "Message.hpp"
#include <string>
#include <memory>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

class NetworkFunction {
public:
    explicit NetworkFunction(NFType type, const std::string& name)
        : type_(type), name_(name), isRunning_(false) {
        instanceId_ = std::to_string(idCounter_++);
    }

    virtual ~NetworkFunction() {
        if (isRunning_) {
            stop();
        }
    }

    NFType getType() const { return type_; }
    std::string getName() const { return name_; }
    std::string getInstanceId() const { return instanceId_; }
    bool getIsRunning() const { return isRunning_; }

    virtual void start() {
        isRunning_ = true;
        Logger::getInstance().info(name_, "Network Function started");
    }

    virtual void stop() {
        isRunning_ = false;
        Logger::getInstance().info(name_, "Network Function stopped");
    }

    virtual void handleMessage(std::shared_ptr<Message> message) = 0;

    void enqueueMessage(std::shared_ptr<Message> message) {
        {
            std::lock_guard<std::mutex> lock(messageMutex_);
            messageQueue_.push(message);
        }
        cv_.notify_one();
    }

    std::shared_ptr<Message> dequeueMessage() {
        std::unique_lock<std::mutex> lock(messageMutex_);
        cv_.wait(lock, [this] { return !messageQueue_.empty(); });
        
        auto msg = messageQueue_.front();
        messageQueue_.pop();
        return msg;
    }

    bool hasMessages() {
        std::lock_guard<std::mutex> lock(messageMutex_);
        return !messageQueue_.empty();
    }

    virtual std::string getStatus() const {
        return name_ + " (" + (isRunning_ ? "Running" : "Stopped") + ")";
    }

protected:
    NFType type_;
    std::string name_;
    std::string instanceId_;
    bool isRunning_;
    
    std::queue<std::shared_ptr<Message>> messageQueue_;
    std::mutex messageMutex_;
    std::condition_variable cv_;

    static uint32_t idCounter_;

    Logger& logger_ = Logger::getInstance();
};

#endif // NETWORK_FUNCTION_HPP
