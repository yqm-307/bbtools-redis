include_directories(
    ${PROJECT_SOURCE_DIR}/src
)

file(GLOB_RECURSE SRCS
    ${PROJECT_SOURCE_DIR}/src/*/*.cc
    ${PROJECT_SOURCE_DIR}/src/*.cc
)



message( ${SRCS} )

add_library(redishelper SHARED ${SRCS})

target_link_libraries(redishelper
    hiredis
    ybbt
)