// There WILL be testing here...
#include "src/script_interpreter.h"
#include "src/util.h"
#include <sstream>

int FAIL_WITH_MSG(const char *str) {
  printf("\nMESSAGE FROM TESTS: %s", str);
  return 1;
}
inline void TEST_MESSAGE(const char *str) {
  printf("\nMESSAGE FROM TESTS: %s", str);
}

bool util_tests() {
  bool testFailed = false;
  int c;
  c = tmx::util::count_string("abcde bede", "de");
  if (c != 2) {
    TEST_MESSAGE("FAILED count_string CHECK 1");
    testFailed = true;
  }
  c = tmx::util::count_string("abcde\n\n\t ab de ab ", "ab");
  if (c != 3) {
    TEST_MESSAGE("FAILED count_string CHECK 2");
    testFailed = true;
  }
  return testFailed;
}
bool interpreter_tests() {

  bool testFailed = false;
  tmx::OneSessionScriptInterpreter tsi;
  const char *infile = "empty.tmx";
  tsi.interpretFile(infile);
  std::stringstream ss;
  tsi.printCommands(ss);
  if (ss.str() != "NO SESSION DETECTED") {
    TEST_MESSAGE("FAILED empty script");
    testFailed = true;
  }

  std::stringstream ss2;
  const char *infile2 = "valid.tmx";
  tsi.interpretFile(infile2);
  tsi.printCommands(ss2);
  std::string ss2str = ss2.str();
  printf("STRING: %s", ss2str.c_str());
  if (ss2str == "NO SESSION DETECTED") {
    TEST_MESSAGE("FAILED valid script 1");
    testFailed = true;
  }
  if (tmx::util::count_string(ss2str, "#!/bin/bash") != 1 ||
      tmx::util::count_string(ss2str, "tmux has-session -t session") != 1 ||
      tmx::util::count_string(ss2str, "if [ $? != 0 ]; then") != 1 ||
      tmx::util::count_string(
          ss2str, "tmux new-session -d -s session -n window") != 1 ||
      tmx::util::count_string(
          ss2str, "tmux send-keys -t session:window 'nvim .' C-m") != 1 ||
      tmx::util::count_string(ss2str, "fi") != 1 ||
      tmx::util::count_string(ss2str, "tmux attach-session -t session") != 1) {
    TEST_MESSAGE("FAILED valid script 2");
    testFailed = true;
  }
  return testFailed;
}

int main() {
  if (util_tests()) {
    return FAIL_WITH_MSG("FAILED UTILITY TESTS, EXITING...");
  }
  if (interpreter_tests()) {
    return FAIL_WITH_MSG("FAILED INTERPRETER TESTS, EXITING...");
  }

  return 0;
}
