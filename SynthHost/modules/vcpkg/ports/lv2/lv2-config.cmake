if(NOT TARGET lv2::lv2)
    get_filename_component(_IMPORT_PREFIX "lv2-config.cmake" PATH)
    get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
    get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)

    add_library(lv2::lv2 INTERFACE IMPORTED)

    set_target_properties(lv2::lv2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
    )

    unset(_IMPORT_PREFIX)
endif()
