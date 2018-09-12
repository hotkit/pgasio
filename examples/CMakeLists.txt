add_executable(pgasio-csj csj.cpp)
target_link_libraries(pgasio-csj f5-pgasio f5-threading)

add_executable(pgasio-minimal minimal.cpp)
target_link_libraries(pgasio-minimal f5-pgasio)

add_executable(pgasio-stats stats.cpp)
target_link_libraries(pgasio-stats f5-pgasio)
