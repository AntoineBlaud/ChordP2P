#pragma once

#include <string>
#include "Socket.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
#include <functional>

void receiveData(Socket * client, string &buffer);
void log_text_periodic(std::string const& text);