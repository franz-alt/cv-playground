set(SANITIZERS "")

option(BUILD_WITH_SANITIZER_ADDRESS "Build all libraries and applications with address sanitizer." OFF)

if(BUILD_WITH_SANITIZER_ADDRESS)
    list(APPEND SANITIZERS "address")
endif()

option(BUILD_WITH_SANITIZER_MEMORY "Build all libraries and applications with memory sanitizer." OFF)

if(BUILD_WITH_SANITIZER_MEMORY)
    list(APPEND SANITIZERS "memory")
endif()

option(BUILD_WITH_SANITIZER_UNDEFINED_BEHAVIOUR "Build all libraries and applications with undefined behaviour sanitizer." OFF)

if(BUILD_WITH_SANITIZER_UNDEFINED_BEHAVIOUR)
    list(APPEND SANITIZERS "undefined")
endif()

list(JOIN SANITIZERS "," LIST_OF_SANITIZERS)
