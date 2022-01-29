find_package(fmt REQUIRED)

salt_interface_library(utils
                       COMMON_TEST "salt/utils/as_local-test.cpp"
                                   "salt/utils/to_string-test.cpp"
                                   "salt/utils/detail/source_location-test.cpp"
                       PUBLIC_LINK fmt::fmt
                                   salt::meta)

# code: language="CMake" insertSpaces=true tabSize=4