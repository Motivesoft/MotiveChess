# CMakeList.txt : CMake project for MotiveChess, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("MotiveChess")

# Add source to this project's executable.
add_executable (MotiveChess "MotiveChess.cpp" "MotiveChess.h" "Engine.cpp" "Engine.h" "Fen.cpp" "Fen.h" "Perft.cpp" "Perft.h" "Board.cpp" "Board.h" "Move.cpp" "Move.h" "BitBoard.cpp" "BitBoard.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MotiveChess PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
