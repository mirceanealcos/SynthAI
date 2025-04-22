vcpkg_download_distfile(ARCHIVE
    URLS "https://www.inf.puc-rio.br/~roberto/lpeg/lpeg-${VERSION}.tar.gz"
    FILENAME "lpeg-${VERSION}.tar.gz"
    SHA512 01b2a4ceb2d110e143603bc63c84a59736ea735dd0ed9866286ba115d41be48d09c9ff21c8e2327974d2296944f6508d50a5c3a18f26ac1d81b8b2fc41f61222
)

vcpkg_extract_source_archive(
    SOURCE_PATH
    ARCHIVE "${ARCHIVE}"
)

file(COPY "CMakeLists.txt" DESTINATION "${SOURCE_PATH}")
file(COPY "lpeg.def" DESTINATION "${SOURCE_PATH}")
file(COPY "unofficial-lpeg-config.cmake.in" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        "-DLPEG_VERSION=${VERSION}"
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "unofficial-lpeg")

# Remove debug share
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

# Handle copyright
file(INSTALL "${SOURCE_PATH}/lpeg.html" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
file(INSTALL "usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")

# Allow empty include directory
set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)
