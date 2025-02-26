set(ALL_SOURCES "${CMAKE_SOURCE_DIR}/src")

macro(create_filter GROUP_NAME)
    set(FILES "")
    foreach(FILE ${ARGN})
        list(APPEND FILES "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}")
    endforeach()

    # Append files to the global ALL_SOURCES list
    list(APPEND ALL_SOURCES ${FILES})

    # Apply the filter in Visual Studio
    source_group("${GROUP_NAME}" FILES ${FILES})
endmacro()

set(ALL_SOURCES "")

create_filter(0.Types
    vertex.h
    vertex.cpp)

create_filter(1.Application
    application.h
    application.cpp)

target_sources(VulkanEngine PRIVATE ${ALL_SOURCES})
