salt_interface_library(events
    COMMON
        TEST
            "salt/events/event_system-test.cpp"
        LINK
            salt::math)

# code: language="CMake" insertSpaces=true tabSize=4