add_executable(pgasio-csj csj.cpp)
target_link_libraries(pgasio-csj f5-pgasio f5-threading)
install(TARGETS pgasio-csj EXPORT pgasio-csj RUNTIME DESTINATION bin)

add_executable(pgasio-minimal minimal.cpp)
target_link_libraries(pgasio-minimal f5-pgasio)
install(TARGETS pgasio-minimal EXPORT pgasio-minimal RUNTIME DESTINATION bin)

add_executable(pgasio-stats stats.cpp)
target_link_libraries(pgasio-stats f5-pgasio)
install(TARGETS pgasio-stats EXPORT pgasio-stats RUNTIME DESTINATION bin)
