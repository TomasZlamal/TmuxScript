// special words: ["session", "window", "select", "attach", "split",
// "vertically", "horizontally"]

// special word-letters: ["{", "}"]
#include "src/script_interpreter.h"

int main(int argc, char **argv) {
  tmx::OneSessionScriptInterpreter tsi;
  const char *infile = argv[1];
  tsi.interpretFile(infile);
  std::ofstream outfile;
  outfile.open(".tmx_bash");
  tsi.printCommands(outfile);
  outfile.close();
  system("bash .tmx_bash");
  return 0;
}
