#include "script_interpreter.h"
#include "util.h"
namespace tmx {

void OneSessionScriptInterpreter::_processWord(const std::string &word) {
  m_State.instruction_stack.push(word.c_str());

  VERBOSE_DEBUG_PRINT(word)
  if (m_State.reading_command_length >
      -1) { // we are reading a command right now

    ++m_State.reading_command_length;
    if (util::count_char(word, '}') != 0) {
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
  if (util::count_char(word, '"') == 2) {
    std::string name =
        word.substr(word.find_first_of("\"") + 1, word.find_last_of("\"") - 1);

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

    if (util::count_string(word, "#") == 1) {
      out_commands.push_back(BashCommand{
          CommandType::SelectWindow, name, m_State.current_session, {}});
    }
  }

  if (util::count_string(word, "!") == 1) {
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
void OneSessionScriptInterpreter::interpretFile(const char *infile) {
  std::ifstream file(infile);
  m_State.reading_command_length = -1;
  m_State.current_session = "";
  m_State.current_window = "";

  std::string word;
  while (file >> word) {
    m_State.words.push_back(word);
  }
  for (const std::string &word : m_State.words) {
    _processWord(word);
  }
}
void OneSessionScriptInterpreter::printCommands(std::ostream &out) {
  if (m_State.current_session == "") {
    out << "NO SESSION DETECTED";
    return;
  }
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
            << " -n " << command.value << "\n";
      } else {
        out << "\ttmux new-window -t " << command.for_session.value() << " -n "
            << command.value << "\n";
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
  if (m_State.current_session != "" && selected_window != "")
    out << "\ttmux select-window -t " << m_State.current_session << ":"
        << selected_window << "\n";
  out << "fi\n";

  if (m_State.current_session != "")
    out << "tmux attach-session -t " << m_State.current_session;
}

} // namespace tmx
