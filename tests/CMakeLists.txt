# Find GTest from the system (installed in Docker builder image)
find_package(GTest REQUIRED)

add_subdirectory(unit)
# add_subdirectory(integration) #TODO: enable once ready
