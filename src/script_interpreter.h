#pragma once
#include "command.h"
#include <fstream>
#include <iostream>
#include <optional>
#include <stack>
#include <string>
#include <vector>

#define DEBUG 1
#if DEBUG >= 1
#define DEBUG_PRINT(x) std::clog << x << std::flush;
#else
#define DEBUG_PRINT(x)
#endif

#if DEBUG >= 2
#define VERBOSE_DEBUG_PRINT(x) std::clog << x << std::flush;
#else
#define VERBOSE_DEBUG_PRINT(x)
#endif

namespace tmx {
// One Session type
class OneSessionScriptInterpreter {
private:
  // util functions
  std::vector<BashCommand> out_commands;
  struct State {
    std::vector<std::string> words;
    std::string current_window;
    std::string current_session;
    std::stack<std::string> instruction_stack;
    int reading_command_length; // how many of the last instructions are a part
                                // of the command? -1 = not reading a command
                                // >-1 = reading a command
  } m_State;

  void _processWord(const std::string &word);

public:
  void interpretFile(const char *infile);
  void printCommands(std::ostream &out);
};
} // namespace tmx
