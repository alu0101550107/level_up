add_library(levelup_core STATIC
  src/Database.cpp
  src/DateUtils.cpp
  src/EventRepository.cpp
)
target_include_directories(levelup_core PUBLIC include)
target_link_libraries(levelup_core PUBLIC levelup_sqlite3)

if(NOT ANDROID)
  add_executable(levelup_date_utils_tests tests/test_date_utils.cpp)
  target_link_libraries(levelup_date_utils_tests PRIVATE levelup_core)
  add_test(NAME date_utils_tests COMMAND levelup_date_utils_tests)

  add_executable(levelup_database_tests tests/test_database.cpp)
  target_link_libraries(levelup_database_tests PRIVATE levelup_core)
  add_test(NAME database_tests COMMAND levelup_database_tests)

  add_executable(levelup_event_repository_tests tests/test_event_repository.cpp)
  target_link_libraries(levelup_event_repository_tests PRIVATE levelup_core)
  add_test(NAME event_repository_tests COMMAND levelup_event_repository_tests)
endif()
