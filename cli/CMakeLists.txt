# Use the package PkgConfig to detect GTK+ headers/library files
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

# Add other flags to the compiler
add_definitions(${GTK3_CFLAGS_OTHER})

################################################################
# compile register app
# include_directories(..)
add_executable(register_application
  register_application.cc
  global.cc
  gui.cc
  log.cc
  utils.cc

  register_app.cc
  register_gui.cc
  register_client.cc
  register_global.cc

  # # gui.cc ->
  # chat_gui.cc # chat_gui.cc -> rainbow_client.cc
  # rainbow_client.cc
  # rainbow_data.cc # chat_gui.cc -> rainbow_data.cc
  # rainbow_global.cc # -> rainbow_gui.cc
  # rainbow_gui.cc

  # ..
  ../app.cc
  ../protocol.cc
  ../packet_manager.cc
  ../user_data.cc
  ../user_manager.cc
  ../session_manager.cc
  )
target_link_libraries(register_application muduo_net)
target_link_libraries(register_application muduo_extra)
target_link_libraries(register_application ${GTK3_LIBRARIES})


################################################################
# compile register app
# include_directories(..)
add_executable(rainbow
  rainbow.cc
  global.cc
  gui.cc
  log.cc
  utils.cc
  chat_gui.cc
  rainbow_gui.cc
  rainbow_app.cc
  rainbow_client.cc
  rainbow_data.cc
  rainbow_global.cc

  # ..
  ../app.cc
  ../protocol.cc
  ../packet_manager.cc
  ../user_data.cc
  ../user_manager.cc
  ../session_manager.cc
  )

target_link_libraries(rainbow muduo_net)
target_link_libraries(rainbow muduo_extra)
target_link_libraries(rainbow ${GTK3_LIBRARIES})
