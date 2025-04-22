file(READ "usage" usage)
message(WARNING "find_package(unofficial-libargon2) is deprecated.\n${usage}")
include(CMakeFindDependencyMacro)
find_dependency(unofficial-argon2 CONFIG)
