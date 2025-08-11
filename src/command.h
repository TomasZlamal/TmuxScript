#pragma once
#include <optional>
#include <string>

namespace tmx {
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
} // namespace tmx
