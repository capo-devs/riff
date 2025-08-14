file(ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_SOURCE_DIR}/src.zip" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")

if(RIFF_MA_DEBUG_OUTPUT)
  set(CAPO_MA_DEBUG_OUTPUT ON)
endif()

message(STATUS "[klib]")
add_subdirectory(src/klib)

message(STATUS "[capo-lite]")
add_subdirectory(src/capo_lite)

message(STATUS "[gvdi]")
add_subdirectory(src/gvdi)

message(STATUS "[icons-kenny]")
add_subdirectory(src/icons_kenny)

if(RIFF_BUILD_BIN2CPP)
  add_subdirectory(src/bin2cpp)
endif()
