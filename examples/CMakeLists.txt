add_executable(pg-asio-csj csj.cpp)
target_link_libraries(pg-asio-csj f5-pgasio f5-threading)

add_executable(pg-asio-minimal minimal.cpp)
target_link_libraries(pg-asio-minimal f5-pgasio)

add_executable(pg-asio-stats stats.cpp)
target_link_libraries(pg-asio-stats f5-pgasio)
