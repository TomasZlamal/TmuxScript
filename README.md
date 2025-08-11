# Introduction
This is a mini-language that allows you to quickly setup Tmux in your project.
## Bash vs Tmx comparison
### Bash
Instead of having to write:
```bash
#!/bin/bash
tmux has-session -t cppray
if [ $? != 0 ]; then
	tmux new-session -d -s cppray -n nvim
	tmux send-keys -t cppray:nvim 'cd ~/code/projects/cppraytracer/' C-m
	tmux send-keys -t cppray:nvim 'nvim .' C-m
	tmux new-window -t cppray -n cmake
	tmux send-keys -t cppray:cmake 'cd ~/code/projects/cppraytracer/build/debug/' C-m
	tmux select-window -t cppray:nvim
fi
tmux attach-session -t cppray
```
### Tmx
You can just write:
```
"cppray"
  "nvim"#
    !{nvim .}

  "testing"
    !{cd test}

  "debug"
    {cd build/debug}
```
Note that whitespace does not matter (beyond seperating "words" by a space).
The "#" means "select this window". It's optional.
The first string is always interpreted as the session name. All other strings are individual windows.

This example opens a session called "cppray", with three windows.
`!{}` executes some command inside of some window. The Interpreter assumes you want to attach to the session, so it auto-attached to the session.
## Building and running it
Create a build directory and build it:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
You should get a `tmuxscript` executable in the build directory. You can optionally rename this to `tmx`.
Put the executable in your system PATH. Now you can use it using `tmx script.tmx`.
## Testing
Testing is set up with CMake, so it works similiarly to building.
```bash
mkdit test
cd test
cmake ..
cmake --build .
ctest
```
You should see that all tests pass. Note that all code for tests is inside of `test.cc`
