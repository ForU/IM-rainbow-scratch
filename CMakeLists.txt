# executable
add_executable(rainbowserver rainbowserver.cc
  app.cc
  log.cc
  configure.cc
  protocol.cc
  packet_manager.cc
  server_app.cc
  server_core.cc
  session_manager.cc
  user_data.cc
  user_manager.cc
  )
target_link_libraries(rainbowserver muduo_net)
target_link_libraries(rainbowserver muduo_extra)


# # test_app_user_manager
# add_executable(test_app_user_manager test_app_user_manager.cc packet_manager.cc user_manager.cc user_data.cc session_manager.cc)
# target_link_libraries(test_app_user_manager muduo_net)
# target_link_libraries(test_app_user_manager muduo_extra)


# client
add_subdirectory(cli)
add_subdirectory(test)
