# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /root/c0-vm-cpp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/c0-vm-cpp/build

# Include any dependencies generated for this target.
include src/CMakeFiles/LIB_SRC.dir/depend.make

# Include the progress variables for this target.
include src/CMakeFiles/LIB_SRC.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/LIB_SRC.dir/flags.make

src/CMakeFiles/LIB_SRC.dir/file.cpp.o: src/CMakeFiles/LIB_SRC.dir/flags.make
src/CMakeFiles/LIB_SRC.dir/file.cpp.o: ../src/file.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/c0-vm-cpp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/LIB_SRC.dir/file.cpp.o"
	cd /root/c0-vm-cpp/build/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/LIB_SRC.dir/file.cpp.o -c /root/c0-vm-cpp/src/file.cpp

src/CMakeFiles/LIB_SRC.dir/file.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/LIB_SRC.dir/file.cpp.i"
	cd /root/c0-vm-cpp/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/c0-vm-cpp/src/file.cpp > CMakeFiles/LIB_SRC.dir/file.cpp.i

src/CMakeFiles/LIB_SRC.dir/file.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/LIB_SRC.dir/file.cpp.s"
	cd /root/c0-vm-cpp/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/c0-vm-cpp/src/file.cpp -o CMakeFiles/LIB_SRC.dir/file.cpp.s

src/CMakeFiles/LIB_SRC.dir/vm.cpp.o: src/CMakeFiles/LIB_SRC.dir/flags.make
src/CMakeFiles/LIB_SRC.dir/vm.cpp.o: ../src/vm.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/c0-vm-cpp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/LIB_SRC.dir/vm.cpp.o"
	cd /root/c0-vm-cpp/build/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/LIB_SRC.dir/vm.cpp.o -c /root/c0-vm-cpp/src/vm.cpp

src/CMakeFiles/LIB_SRC.dir/vm.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/LIB_SRC.dir/vm.cpp.i"
	cd /root/c0-vm-cpp/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/c0-vm-cpp/src/vm.cpp > CMakeFiles/LIB_SRC.dir/vm.cpp.i

src/CMakeFiles/LIB_SRC.dir/vm.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/LIB_SRC.dir/vm.cpp.s"
	cd /root/c0-vm-cpp/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/c0-vm-cpp/src/vm.cpp -o CMakeFiles/LIB_SRC.dir/vm.cpp.s

# Object files for target LIB_SRC
LIB_SRC_OBJECTS = \
"CMakeFiles/LIB_SRC.dir/file.cpp.o" \
"CMakeFiles/LIB_SRC.dir/vm.cpp.o"

# External object files for target LIB_SRC
LIB_SRC_EXTERNAL_OBJECTS =

src/libLIB_SRC.a: src/CMakeFiles/LIB_SRC.dir/file.cpp.o
src/libLIB_SRC.a: src/CMakeFiles/LIB_SRC.dir/vm.cpp.o
src/libLIB_SRC.a: src/CMakeFiles/LIB_SRC.dir/build.make
src/libLIB_SRC.a: src/CMakeFiles/LIB_SRC.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/c0-vm-cpp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libLIB_SRC.a"
	cd /root/c0-vm-cpp/build/src && $(CMAKE_COMMAND) -P CMakeFiles/LIB_SRC.dir/cmake_clean_target.cmake
	cd /root/c0-vm-cpp/build/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/LIB_SRC.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/LIB_SRC.dir/build: src/libLIB_SRC.a

.PHONY : src/CMakeFiles/LIB_SRC.dir/build

src/CMakeFiles/LIB_SRC.dir/clean:
	cd /root/c0-vm-cpp/build/src && $(CMAKE_COMMAND) -P CMakeFiles/LIB_SRC.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/LIB_SRC.dir/clean

src/CMakeFiles/LIB_SRC.dir/depend:
	cd /root/c0-vm-cpp/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/c0-vm-cpp /root/c0-vm-cpp/src /root/c0-vm-cpp/build /root/c0-vm-cpp/build/src /root/c0-vm-cpp/build/src/CMakeFiles/LIB_SRC.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/LIB_SRC.dir/depend

