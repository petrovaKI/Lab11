// Copyright 2022 Petrova Kseniya <petrovaKI>

#ifndef INCLUDE_MY_CMAKE_HPP_
#define INCLUDE_MY_CMAKE_HPP_

#include <async++.h>
#include <boost/process.hpp>
#include <boost/process/extend.hpp>
#include <boost/program_options.hpp>
#include <vector>
#include <iostream>
#include <signal.h>
#include <string>
#include <thread>
#include <chrono>

namespace processes = boost::process;
namespace po = boost::program_options;

void start_cmake(int argc, char* argv[]);

void create_child(const std::string& command, const time_t& period);

void create_child(const std::string& command, const time_t& period, int& res);

void check_time(processes::child& process, const time_t& period);

time_t time_now();

#endif // INCLUDE_MY_CMAKE_HPP_
