# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.30.4/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.30.4/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/ellenclarke/Desktop/lab4

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/ellenclarke/Desktop/lab4/cmake-build-debug

# Include any dependencies generated for this target.
include external/glad-3.3/CMakeFiles/glad.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include external/glad-3.3/CMakeFiles/glad.dir/compiler_depend.make

# Include the progress variables for this target.
include external/glad-3.3/CMakeFiles/glad.dir/progress.make

# Include the compile flags for this target's objects.
include external/glad-3.3/CMakeFiles/glad.dir/flags.make

external/glad-3.3/CMakeFiles/glad.dir/src/gl.c.o: external/glad-3.3/CMakeFiles/glad.dir/flags.make
external/glad-3.3/CMakeFiles/glad.dir/src/gl.c.o: /Users/ellenclarke/Desktop/lab4/external/glad-3.3/src/gl.c
external/glad-3.3/CMakeFiles/glad.dir/src/gl.c.o: external/glad-3.3/CMakeFiles/glad.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/ellenclarke/Desktop/lab4/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object external/glad-3.3/CMakeFiles/glad.dir/src/gl.c.o"
	cd /Users/ellenclarke/Desktop/lab4/cmake-build-debug/external/glad-3.3 && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT external/glad-3.3/CMakeFiles/glad.dir/src/gl.c.o -MF CMakeFiles/glad.dir/src/gl.c.o.d -o CMakeFiles/glad.dir/src/gl.c.o -c /Users/ellenclarke/Desktop/lab4/external/glad-3.3/src/gl.c

external/glad-3.3/CMakeFiles/glad.dir/src/gl.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/glad.dir/src/gl.c.i"
	cd /Users/ellenclarke/Desktop/lab4/cmake-build-debug/external/glad-3.3 && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/ellenclarke/Desktop/lab4/external/glad-3.3/src/gl.c > CMakeFiles/glad.dir/src/gl.c.i

external/glad-3.3/CMakeFiles/glad.dir/src/gl.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/glad.dir/src/gl.c.s"
	cd /Users/ellenclarke/Desktop/lab4/cmake-build-debug/external/glad-3.3 && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/ellenclarke/Desktop/lab4/external/glad-3.3/src/gl.c -o CMakeFiles/glad.dir/src/gl.c.s

# Object files for target glad
glad_OBJECTS = \
"CMakeFiles/glad.dir/src/gl.c.o"

# External object files for target glad
glad_EXTERNAL_OBJECTS =

libglad.a: external/glad-3.3/CMakeFiles/glad.dir/src/gl.c.o
libglad.a: external/glad-3.3/CMakeFiles/glad.dir/build.make
libglad.a: external/glad-3.3/CMakeFiles/glad.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/ellenclarke/Desktop/lab4/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library ../../libglad.a"
	cd /Users/ellenclarke/Desktop/lab4/cmake-build-debug/external/glad-3.3 && $(CMAKE_COMMAND) -P CMakeFiles/glad.dir/cmake_clean_target.cmake
	cd /Users/ellenclarke/Desktop/lab4/cmake-build-debug/external/glad-3.3 && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/glad.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
external/glad-3.3/CMakeFiles/glad.dir/build: libglad.a
.PHONY : external/glad-3.3/CMakeFiles/glad.dir/build

external/glad-3.3/CMakeFiles/glad.dir/clean:
	cd /Users/ellenclarke/Desktop/lab4/cmake-build-debug/external/glad-3.3 && $(CMAKE_COMMAND) -P CMakeFiles/glad.dir/cmake_clean.cmake
.PHONY : external/glad-3.3/CMakeFiles/glad.dir/clean

external/glad-3.3/CMakeFiles/glad.dir/depend:
	cd /Users/ellenclarke/Desktop/lab4/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/ellenclarke/Desktop/lab4 /Users/ellenclarke/Desktop/lab4/external/glad-3.3 /Users/ellenclarke/Desktop/lab4/cmake-build-debug /Users/ellenclarke/Desktop/lab4/cmake-build-debug/external/glad-3.3 /Users/ellenclarke/Desktop/lab4/cmake-build-debug/external/glad-3.3/CMakeFiles/glad.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : external/glad-3.3/CMakeFiles/glad.dir/depend

