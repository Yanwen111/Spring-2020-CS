# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_COMMAND = /snap/clion/114/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/114/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug"

# Include any dependencies generated for this target.
include source/Ultrasound/CMakeFiles/ULTRASOUND.dir/depend.make

# Include the progress variables for this target.
include source/Ultrasound/CMakeFiles/ULTRASOUND.dir/progress.make

# Include the compile flags for this target's objects.
include source/Ultrasound/CMakeFiles/ULTRASOUND.dir/flags.make

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/data.cpp.o: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/flags.make
source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/data.cpp.o: ../source/Ultrasound/source/data.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/data.cpp.o"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ULTRASOUND.dir/source/data.cpp.o -c "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/data.cpp"

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/data.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ULTRASOUND.dir/source/data.cpp.i"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/data.cpp" > CMakeFiles/ULTRASOUND.dir/source/data.cpp.i

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/data.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ULTRASOUND.dir/source/data.cpp.s"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/data.cpp" -o CMakeFiles/ULTRASOUND.dir/source/data.cpp.s

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/gui.cpp.o: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/flags.make
source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/gui.cpp.o: ../source/Ultrasound/source/gui.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/gui.cpp.o"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ULTRASOUND.dir/source/gui.cpp.o -c "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/gui.cpp"

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/gui.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ULTRASOUND.dir/source/gui.cpp.i"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/gui.cpp" > CMakeFiles/ULTRASOUND.dir/source/gui.cpp.i

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/gui.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ULTRASOUND.dir/source/gui.cpp.s"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/gui.cpp" -o CMakeFiles/ULTRASOUND.dir/source/gui.cpp.s

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/helper.cpp.o: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/flags.make
source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/helper.cpp.o: ../source/Ultrasound/source/helper.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/helper.cpp.o"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ULTRASOUND.dir/source/helper.cpp.o -c "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/helper.cpp"

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/helper.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ULTRASOUND.dir/source/helper.cpp.i"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/helper.cpp" > CMakeFiles/ULTRASOUND.dir/source/helper.cpp.i

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/helper.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ULTRASOUND.dir/source/helper.cpp.s"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/helper.cpp" -o CMakeFiles/ULTRASOUND.dir/source/helper.cpp.s

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/marker.cpp.o: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/flags.make
source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/marker.cpp.o: ../source/Ultrasound/source/marker.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/marker.cpp.o"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ULTRASOUND.dir/source/marker.cpp.o -c "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/marker.cpp"

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/marker.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ULTRASOUND.dir/source/marker.cpp.i"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/marker.cpp" > CMakeFiles/ULTRASOUND.dir/source/marker.cpp.i

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/marker.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ULTRASOUND.dir/source/marker.cpp.s"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/marker.cpp" -o CMakeFiles/ULTRASOUND.dir/source/marker.cpp.s

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/probe.cpp.o: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/flags.make
source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/probe.cpp.o: ../source/Ultrasound/source/probe.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/probe.cpp.o"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ULTRASOUND.dir/source/probe.cpp.o -c "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/probe.cpp"

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/probe.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ULTRASOUND.dir/source/probe.cpp.i"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/probe.cpp" > CMakeFiles/ULTRASOUND.dir/source/probe.cpp.i

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/probe.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ULTRASOUND.dir/source/probe.cpp.s"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/probe.cpp" -o CMakeFiles/ULTRASOUND.dir/source/probe.cpp.s

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/remote.cpp.o: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/flags.make
source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/remote.cpp.o: ../source/Ultrasound/source/remote.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/remote.cpp.o"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ULTRASOUND.dir/source/remote.cpp.o -c "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/remote.cpp"

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/remote.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ULTRASOUND.dir/source/remote.cpp.i"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/remote.cpp" > CMakeFiles/ULTRASOUND.dir/source/remote.cpp.i

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/remote.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ULTRASOUND.dir/source/remote.cpp.s"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/remote.cpp" -o CMakeFiles/ULTRASOUND.dir/source/remote.cpp.s

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/rotation.cpp.o: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/flags.make
source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/rotation.cpp.o: ../source/Ultrasound/source/rotation.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/rotation.cpp.o"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ULTRASOUND.dir/source/rotation.cpp.o -c "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/rotation.cpp"

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/rotation.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ULTRASOUND.dir/source/rotation.cpp.i"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/rotation.cpp" > CMakeFiles/ULTRASOUND.dir/source/rotation.cpp.i

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/rotation.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ULTRASOUND.dir/source/rotation.cpp.s"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/rotation.cpp" -o CMakeFiles/ULTRASOUND.dir/source/rotation.cpp.s

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/scale.cpp.o: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/flags.make
source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/scale.cpp.o: ../source/Ultrasound/source/scale.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/scale.cpp.o"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ULTRASOUND.dir/source/scale.cpp.o -c "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/scale.cpp"

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/scale.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ULTRASOUND.dir/source/scale.cpp.i"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/scale.cpp" > CMakeFiles/ULTRASOUND.dir/source/scale.cpp.i

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/scale.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ULTRASOUND.dir/source/scale.cpp.s"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound/source/scale.cpp" -o CMakeFiles/ULTRASOUND.dir/source/scale.cpp.s

# Object files for target ULTRASOUND
ULTRASOUND_OBJECTS = \
"CMakeFiles/ULTRASOUND.dir/source/data.cpp.o" \
"CMakeFiles/ULTRASOUND.dir/source/gui.cpp.o" \
"CMakeFiles/ULTRASOUND.dir/source/helper.cpp.o" \
"CMakeFiles/ULTRASOUND.dir/source/marker.cpp.o" \
"CMakeFiles/ULTRASOUND.dir/source/probe.cpp.o" \
"CMakeFiles/ULTRASOUND.dir/source/remote.cpp.o" \
"CMakeFiles/ULTRASOUND.dir/source/rotation.cpp.o" \
"CMakeFiles/ULTRASOUND.dir/source/scale.cpp.o"

# External object files for target ULTRASOUND
ULTRASOUND_EXTERNAL_OBJECTS =

source/Ultrasound/libULTRASOUND.a: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/data.cpp.o
source/Ultrasound/libULTRASOUND.a: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/gui.cpp.o
source/Ultrasound/libULTRASOUND.a: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/helper.cpp.o
source/Ultrasound/libULTRASOUND.a: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/marker.cpp.o
source/Ultrasound/libULTRASOUND.a: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/probe.cpp.o
source/Ultrasound/libULTRASOUND.a: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/remote.cpp.o
source/Ultrasound/libULTRASOUND.a: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/rotation.cpp.o
source/Ultrasound/libULTRASOUND.a: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/source/scale.cpp.o
source/Ultrasound/libULTRASOUND.a: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/build.make
source/Ultrasound/libULTRASOUND.a: source/Ultrasound/CMakeFiles/ULTRASOUND.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_9) "Linking CXX static library libULTRASOUND.a"
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && $(CMAKE_COMMAND) -P CMakeFiles/ULTRASOUND.dir/cmake_clean_target.cmake
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ULTRASOUND.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
source/Ultrasound/CMakeFiles/ULTRASOUND.dir/build: source/Ultrasound/libULTRASOUND.a

.PHONY : source/Ultrasound/CMakeFiles/ULTRASOUND.dir/build

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/clean:
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" && $(CMAKE_COMMAND) -P CMakeFiles/ULTRASOUND.dir/cmake_clean.cmake
.PHONY : source/Ultrasound/CMakeFiles/ULTRASOUND.dir/clean

source/Ultrasound/CMakeFiles/ULTRASOUND.dir/depend:
	cd "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS" "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/source/Ultrasound" "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug" "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound" "/home/yanwen/CML_CS/Summer 2020/Spring-2020-CS/cmake-build-debug/source/Ultrasound/CMakeFiles/ULTRASOUND.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : source/Ultrasound/CMakeFiles/ULTRASOUND.dir/depend
