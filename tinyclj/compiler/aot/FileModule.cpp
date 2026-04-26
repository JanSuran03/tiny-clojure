#include <fstream>

#include "FileModule.h"

FileModule::FileModule(std::string name,
                       std::unordered_set<std::string> imports)
        : Module(std::move(name), std::move(imports)) {}
