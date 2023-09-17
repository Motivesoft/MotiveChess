// MotiveChess.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "Engine.h"

// Methods
bool processCommandLine( Engine& engine, const std::string& switchPrefix, const std::vector<std::string>& args );

