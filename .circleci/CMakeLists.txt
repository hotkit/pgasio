cmake_minimum_required(VERSION 3.10)
project(pgasio-circleci)

# Opt in to testing
add_custom_target(check)

add_subdirectory(boost)
add_subdirectory(.. pgasio)
add_subdirectory(../examples pgasio/examples)
add_subdirectory(threading)
