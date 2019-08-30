#pragma once

#include <string>
#include <vector>

std::string displayMenu(std::vector<std::string> options);
std::string displayMenu(std::string title, std::vector <std::string> options);
std::string displayMenu(std::string title, std::vector <std::string> options, std::string footer);