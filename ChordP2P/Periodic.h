#pragma once

#include <ctime>
#include <iostream>
#include <iomanip>
#include <functional>
#include <string>
#include "common.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>


class PeriodicTask : boost::noncopyable
{
public:
    typedef std::function<void()> handler_fn;

    PeriodicTask(boost::asio::io_service& ioService
        , std::string const& name
        , int interval
        , handler_fn task)
        : ioService(ioService)
        , interval(interval)
        , task(task)
        , name(name)
        , timer(ioService)
    {
        //log_text_periodic("Create PeriodicTask '" + name + "'");
        // Schedule start to be ran by the io_service
        ioService.post(boost::bind(&PeriodicTask::start, this));
    }

    void execute(boost::system::error_code const& e)
    {
        if (e != boost::asio::error::operation_aborted) {
            //log_text_periodic("Execute PeriodicTask '" + name + "'");

            task();

            timer.expires_at(timer.expires_at() + boost::posix_time::seconds(interval));
            start_wait();
        }
    }

    void start()
    {
        //log_text_periodic("Start PeriodicTask '" + name + "'");

        // Uncomment if you want to call the handler on startup (i.e. at time 0)
        // task();

        timer.expires_from_now(boost::posix_time::seconds(interval));
        start_wait();
    }

private:
    void start_wait()
    {
        timer.async_wait(boost::bind(&PeriodicTask::execute
            , this
            , boost::asio::placeholders::error));
    }

private:
    boost::asio::io_service& ioService;
    boost::asio::deadline_timer timer;
    handler_fn task;
    std::string name;
    int interval;
};

class PeriodicScheduler : boost::noncopyable
{
public:
    void run()
    {
        io_service.run();
    }

    void addTask(std::string const& name
        , PeriodicTask::handler_fn const& task
        , int interval)
    {
        tasks.push_back(std::make_unique<PeriodicTask>(std::ref(io_service)
            , name, interval, task));
    }

private:
    boost::asio::io_service io_service;
    std::vector<std::unique_ptr<PeriodicTask>> tasks;
};