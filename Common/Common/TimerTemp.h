#pragma once

#include <chrono>

#define START_TIMER(aVariableName) std::chrono::high_resolution_clock::time_point aVariableName = std::chrono::high_resolution_clock::now();
#define END_TIMER_GET_RESULT_MS(aVariableName) /*std::chrono::duration_cast<std::chrono::milliseconds>*/std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - aVariableName).count();