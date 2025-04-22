set(VCPKG_POLICY_EMPTY_PACKAGE enabled)

configure_file("vcpkg-cmake-wrapper.cmake" "${CURRENT_PACKAGES_DIR}/share/egl/vcpkg-cmake-wrapper.cmake" @ONLY)

if(VCPKG_TARGET_IS_WINDOWS)
    configure_file("egl.pc.in" "${CURRENT_PACKAGES_DIR}/lib/pkgconfig/egl.pc" @ONLY)
    if (NOT VCPKG_BUILD_TYPE)
        configure_file("egl.pc.in" "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig/egl.pc" @ONLY)
    endif()
    vcpkg_fixup_pkgconfig()
endif()
