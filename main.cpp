#include <functional>
#include <iostream>
#include <utility>

#include "runtime/Runtime.h"
#include "types/TCInteger.h"

struct CLIOption {
    std::string m_Mame;
    std::string m_Description;
    std::string m_DefaultValue;
    std::function<void(const std::string &)> m_Handler;

    CLIOption(std::string name,
              std::string description,
              std::string defaultValue,
              std::function<void(const std::string &)> handler)
            : m_Mame(std::move(name)),
              m_Description(std::move(description)),
              m_DefaultValue(std::move(defaultValue)),
              m_Handler(std::move(handler)) {}
};

std::vector<CLIOption> cli_options = {
        CLIOption("help", "Shows this help message", "", [](const std::string &) {
            std::cout << "You can pass command-line options using --option or -option value syntax." << std::endl
                      << "where --option is the same as -option true." << std::endl
                      << "Individual CLI options are separated by spaces." << std::endl
                      << "Available command-line options:" << std::endl;
            for (const auto &option: cli_options) {
                std::cout << "  --" << option.m_Mame;
                if (!option.m_DefaultValue.empty()) {
                    std::cout << " (default: " << option.m_DefaultValue << ")";
                }
                std::cout << ": " << option.m_Description << std::endl;
            }
        }),
        CLIOption("suppress-repl-welcome", "Suppresses the welcome message when starting the REPL", "false",
                  [](const std::string &value) {
                      if (value == "true") {
                          Runtime::getInstance().m_SuppressReplWelcome = true;
                      }
                  }),
        CLIOption("disable-gc", "Disables garbage collection (not recommended)", "false",
                  [](const std::string &value) {
                      if (value == "true") {
                          Runtime::getInstance().m_DisableGC = true;
                      }
                  }),
        CLIOption("opt-level", "Sets the optimization level for AOT compilation (O0, O1, O2, O3)", "O0",
                  [](const std::string &value) {
                      if (value == "O0") {
                          Runtime::getInstance().getAotEngine().m_OptimizationLevel = llvm::OptimizationLevel::O0;
                      } else if (value == "O1") {
                          Runtime::getInstance().getAotEngine().m_OptimizationLevel = llvm::OptimizationLevel::O1;
                      } else if (value == "O2") {
                          Runtime::getInstance().getAotEngine().m_OptimizationLevel = llvm::OptimizationLevel::O2;
                      } else if (value == "O3") {
                          Runtime::getInstance().getAotEngine().m_OptimizationLevel = llvm::OptimizationLevel::O3;
                      } else {
                          std::cerr << "Warning: Unrecognized optimization level: " << value
                                    << ". Valid values are O0, O1, O2, O3." << std::endl;
                      }
                  }),
        CLIOption("compiled-dir", "Sets the directory where compiled AOT modules are stored", "compiled",
                  [](const std::string &value) {
                      Runtime::getInstance().getAotEngine().setCompiledDir(value);
                  }),
        CLIOption("direct-linking", "Enables direct linking of AOT-compiled functions (experimental)", "false",
                  [](const std::string &value) {
                      if (value == "true") {
                          Runtime::getInstance().m_DirectLinking = true;
                      }
                  }),
        CLIOption("int-cache-range", "Sets the range of preallocated integers (e.g. -128:127)", "-128:127",
                  [](const std::string &value) {
                      if (value == "off") {
                          TCInteger::st_PreallocationEnabled = false;
                          return;
                      }

                      size_t colon_pos = value.find(':');
                      static constexpr int MIN_RANGE = -65536;
                      static constexpr int MAX_RANGE = 65535;
                      if (colon_pos != std::string::npos) {
                          try {
                              tc_int_t min = std::stoll(value.substr(0, colon_pos));
                              tc_int_t max = std::stoll(value.substr(colon_pos + 1));
                              if (min <= max) {
                                  TCInteger::st_PreallocatedMin = min;
                                  TCInteger::st_PreallocatedMax = max;
                                  TCInteger::init();
                              } else if (min < MIN_RANGE || max > MAX_RANGE) {
                                  std::cerr << "Warning: Integer cache range out of bounds: " << value
                                            << ". Valid range is from " << MIN_RANGE << " to " << MAX_RANGE
                                            << "." << std::endl;
                              } else {
                                  std::cerr << "Warning: Invalid integer cache range: " << value
                                            << ". Min should be less than or equal to max." << std::endl;
                              }
                          } catch (const std::exception &e) {
                              std::cerr << "Warning: Failed to parse integer cache range: " << value
                                        << ". Error: " << e.what() << std::endl;
                          }
                      } else {
                          std::cerr << "Warning: Invalid integer cache range format: " << value
                                    << ". Expected format is min:max (e.g. -128:127)." << std::endl;
                      }
                  }),
        CLIOption("user-code-dir", "Sets the directory where user code is located for AOT compilation", "src",
                  [](const std::string &value) {
                      Runtime::getInstance().getAotEngine().setAdditionalSourceDir(value);
                  }),
};

std::map<std::string, std::string> parse_args(int argc, char *argv[]) {
    std::map<std::string, std::string> args;
    std::optional<std::string> pending_key = std::nullopt;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (pending_key.has_value()) {
            args[pending_key.value()] = arg;
            pending_key = std::nullopt;
        } else if (arg.starts_with("--")) {
            args[arg.substr(2)] = "true";
        } else if (arg.starts_with("-")) {
            pending_key = arg.substr(1);
        } else {
            std::cerr << "Warning: Unrecognized command-line argument: " << arg << std::endl;
        }
    }
    return args;
}

int main(int argc, char *argv[])
try {
    auto args = parse_args(argc, argv);
    if (args.contains("help")) {
        cli_options[0].m_Handler("");
        return 0;
    }
    for (const auto &[key, value]: args) {
        auto option_it = std::find_if(cli_options.begin(), cli_options.end(),
                                      [&key](const CLIOption &option) { return option.m_Mame == key; });
        if (option_it != cli_options.end()) {
            option_it->m_Handler(value);
        } else {
            std::cerr << "Warning: Unrecognized command-line option: " << key << std::endl;
        }
    }

    Runtime::init();

    Runtime::repl();
    return 0;
} catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
}
