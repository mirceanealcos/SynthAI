set(VCPKG_POLICY_EMPTY_PACKAGE enabled)

if(NOT TARGET_TRIPLET STREQUAL _HOST_TRIPLET)
    message(FATAL_ERROR "vcpkg-gn is a host-only port; please mark it as a host port in your dependencies.")
endif()

file(INSTALL
    "vcpkg-port-config.cmake"
    "vcpkg_gn_configure.cmake"
    "vcpkg_gn_install.cmake"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
