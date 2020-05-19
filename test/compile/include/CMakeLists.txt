if(TARGET check)
    add_library(pgasio-headers-tests STATIC EXCLUDE_FROM_ALL
            buffered.cpp
            connection.cpp
            errors.cpp
            memory.cpp
            network.cpp
            query.cpp
            record_block.cpp
        )
    target_link_libraries(pgasio-headers-tests f5-pgasio)
    add_dependencies(check pgasio-headers-tests)
endif()
