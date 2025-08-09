// special words: ["session", "window", "select", "attach", "split",
// "vertically", "horizontally"]

// special word-letters: ["{", "}"]
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
enum class CommandType {
  AttachSession,
  SendKeys,
  CreateWindow,
  CreateSessionIfNotExists,
  SelectWindow
};
struct BashCommand {
  CommandType type;
  std::string value;
  std::optional<std::string> for_session;
  std::optional<std::string> for_window;
};

class TmuxScriptInterpreter {
private:
  // util functions
  int f_count_char(const std::string &s, char c) {
    int count = 0;
    bool ignore_next = 0;

    for (int i = 0; i < s.size(); i++) {
      // escape sequence-handling

      if (s[i] == c) {
        if (ignore_next)
          ;
        else {
          count++;
        }
      }
      if (s[i] == '\\')
        ignore_next = 1;
      else
        ignore_next = 0;
    };
    return count;
  }
  int f_count_string(const std::string &haystack, const std::string &needle) {
    if (needle.length() == 0)
      return 0;
    int count = 0;
    for (size_t offset = haystack.find(needle); offset != std::string::npos;
         offset = haystack.find(needle, offset + needle.length())) {
      ++count;
    }
    return count;
  }

private:
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

  void f_processWord(const std::string &word) {
    m_State.instruction_stack.push(word.c_str());
    VERBOSE_DEBUG_PRINT(word)
    if (word == "session" || word == "window" || word == "attach" ||
        word == "select")
      return;
    if (m_State.reading_command_length >
        -1) { // we are reading a command right now

      ++m_State.reading_command_length;
      if (f_count_char(word, '}') != 0) {
        std::string command = "";

        // DEBUG_PRINT(m_State.instruction_stack.top() << "|")
        for (int i = 0; i < m_State.reading_command_length; i++) {
          if (command == "") {
            command = m_State.instruction_stack.top();
          } else
            command = m_State.instruction_stack.top() + " " + command;
          m_State.instruction_stack.pop();
        }
        if (command.find_first_of('}') == command.size() - 1) {
          command = command.substr(0, command.size() - 1);
        }
        out_commands.push_back(BashCommand{CommandType::SendKeys, command,
                                           m_State.current_session,
                                           m_State.current_window});
        /*DEBUG_PRINT(command << "for session" << m_State.current_session
                    << ", window " << m_State.current_window << "\n")*/
        m_State.reading_command_length = -1; // end reading commands
      }
      return;
    }

    // generic string handling
    if (f_count_char(word, '"') == 2) {
      std::string name = word.substr(word.find_first_of("\"") + 1,
                                     word.find_last_of("\"") - 1);
      m_State.instruction_stack.pop();
      std::string instruction = m_State.instruction_stack.top();

      if (instruction == "session") {
        m_State.current_session = name;
        VERBOSE_DEBUG_PRINT("Using session")
        VERBOSE_DEBUG_PRINT(m_State.current_session << "\n")
        out_commands.push_back(
            BashCommand{CommandType::CreateSessionIfNotExists, name, {}, {}});
        // handle using and creating a certain session;
      } else if (instruction == "window") {
        m_State.current_window = name;
        VERBOSE_DEBUG_PRINT("Using window")
        VERBOSE_DEBUG_PRINT(m_State.current_window << "\n")

        out_commands.push_back(BashCommand{
            CommandType::CreateWindow, name, m_State.current_session, {}});
      } else if (instruction == "attach") {
        out_commands.push_back(
            BashCommand{CommandType::AttachSession, name, {}, {}});

      } else if (instruction == "select") {

        out_commands.push_back(BashCommand{
            CommandType::SelectWindow, name, m_State.current_session, {}});
      }
    }
    if (f_count_string(word, "exec") == 1) {
      m_State.reading_command_length = 0;
      std::string command =
          word.substr(word.find_first_of("{") + 1, word.find_first_of("}") - 1);

      VERBOSE_DEBUG_PRINT("Executing command...")
      VERBOSE_DEBUG_PRINT(command)
      // DEBUG_PRINT("TO EXECUTE: " << command << "\n")
      m_State.instruction_stack.pop();
      if (command.size() <= 1)
        ;
      else {
        m_State.instruction_stack.push(command);
        ++m_State.reading_command_length;
      }
      // handle code exection
    }
  }
  void f_processWordSingleSession(const std::string &word) {
    m_State.instruction_stack.push(word.c_str());

    VERBOSE_DEBUG_PRINT(word)
    if (m_State.reading_command_length >
        -1) { // we are reading a command right now

      ++m_State.reading_command_length;
      if (f_count_char(word, '}') != 0) {
        std::string command = "";

        // DEBUG_PRINT(m_State.instruction_stack.top() << "|")
        for (int i = 0; i < m_State.reading_command_length; i++) {
          if (command == "") {
            command = m_State.instruction_stack.top();
          } else
            command = m_State.instruction_stack.top() + " " + command;
          m_State.instruction_stack.pop();
        }
        if (command.find_first_of('}') == command.size() - 1) {
          command = command.substr(0, command.size() - 1);
        }
        out_commands.push_back(BashCommand{CommandType::SendKeys, command,
                                           m_State.current_session,
                                           m_State.current_window});
        /*DEBUG_PRINT(command << "for session" << m_State.current_session
                    << ", window " << m_State.current_window << "\n")*/
        m_State.reading_command_length = -1; // end reading commands
      }
      return;
    }

    // generic string handling
    if (f_count_char(word, '"') == 2) {
      std::string name = word.substr(word.find_first_of("\"") + 1,
                                     word.find_last_of("\"") - 1);

      if (m_State.current_session == "") {
        m_State.current_session = name;
        VERBOSE_DEBUG_PRINT("Using session")
        VERBOSE_DEBUG_PRINT(m_State.current_session << "\n")
        out_commands.push_back(
            BashCommand{CommandType::CreateSessionIfNotExists, name, {}, {}});
        // handle using and creating a certain session;
      } else {
        m_State.current_window = name;
        VERBOSE_DEBUG_PRINT("Using window")
        VERBOSE_DEBUG_PRINT(m_State.current_window << "\n")

        out_commands.push_back(BashCommand{
            CommandType::CreateWindow, name, m_State.current_session, {}});
      }

      if (f_count_string(word, "#") == 1) {
        out_commands.push_back(BashCommand{
            CommandType::SelectWindow, name, m_State.current_session, {}});
      }
    }

    if (f_count_string(word, "!") == 1) {
      m_State.reading_command_length = 0;
      std::string command =
          word.substr(word.find_first_of("{") + 1, word.find_first_of("}") - 1);

      VERBOSE_DEBUG_PRINT("Executing command...")
      VERBOSE_DEBUG_PRINT(command)
      m_State.instruction_stack.pop();
      if (command.size() <= 1)
        ;
      else {
        m_State.instruction_stack.push(command);
        ++m_State.reading_command_length;
      }
      // handle code exection
    }
  }

public:
  void interpretFile(const char *infile) {
    std::ifstream file(infile);
    m_State.reading_command_length = -1;
    m_State.current_session = "";
    m_State.current_window = "";

    std::string word;
    while (file >> word) {
      m_State.words.push_back(word);
    }
    for (const std::string &word : m_State.words) {
      f_processWordSingleSession(word);
    }
  }
  void printCommands(std::ostream &out) {
    out << "#!/bin/bash\n";
    std::string selected_window = "";
    int window_count = 0;
    for (auto &command : out_commands) {
      switch (command.type) {
      case CommandType::CreateSessionIfNotExists: {
        out << "tmux has-session -t " << command.value
            << "\nif [ $? != 0 ]; then\n";
      } break;
      case CommandType::AttachSession: {
        out << "tmux attach-session -t " << command.value << "\n";
      } break;
      case CommandType::CreateWindow: {
        if (window_count == 0) {
          out << "\ttmux new-session -d -s " << command.for_session.value()
              << " -n " << command.value
              << "\n"; /*\ttmux new-window -t "
<< command.for_session.value() << " -n " << command.value << "\n";*/

        } else {
          out << "\ttmux new-window -t " << command.for_session.value()
              << " -n " << command.value << "\n";
        }
        window_count++;
      } break;
      case CommandType::SendKeys: {
        // std::cout << "|" << command.for_window.value() << "|";
        out << "\ttmux send-keys -t " << command.for_session.value() << ":"
            << command.for_window.value() << " '" << command.value << "' C-m\n";
      } break;
      case CommandType::SelectWindow: {
        selected_window = command.value;
      } break;
      }
    }
    out << "\ttmux select-window -t " << m_State.current_session << ":"
        << selected_window << "\n";

    out << "fi\n";
    out << "tmux attach-session -t " << m_State.current_session;
  }
};
int main(int argc, char **argv) {
  TmuxScriptInterpreter tsi;
  const char *infile = argv[1];
  tsi.interpretFile(infile);
  std::ofstream outfile;
  outfile.open(".tmx_bash");
  tsi.printCommands(outfile);
  outfile.close();
  system("bash .tmx_bash");
  return 0;
}
