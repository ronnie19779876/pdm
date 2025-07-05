cmake_minimum_required(VERSION 3.16)

project(PDM VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Gui Network Core5Compat REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Gui Network Core5Compat REQUIRED)
find_package(qca-qt6 REQUIRED)

set(PROJECT_SOURCES
        src/main.cpp
        src/constants.h
        src/basemodel.h
        src/basemodel.cpp
        src/token.h
        src/token.cpp
        src/cluster.h
        src/cluster.cpp
        src/tenant.h
        src/tenant.cpp
        src/namespace.h
        src/namespace.cpp
        src/role.h
        src/role.cpp
        src/producer.h
        src/producer.cpp
        src/consumer.h
        src/consumer.cpp
        src/subscription.h
        src/subscription.cpp
        src/topic.h
        src/topic.cpp
        src/cursor.h
        src/cursor.cpp
        src/basefunction.h
        src/function.h
        src/function.cpp
        src/functioninstance.h
        src/sink.h
        src/sink.cpp
        src/source.h
        src/source.cpp
        src/message.h
        src/message.cpp
        src/pulsarmessage.h
        src/pulsarmessage.cpp
        src/qjsonwebtoken.h
        src/qjsonwebtoken.cpp
        src/qmulticombobox.h
        src/qmulticombobox.cpp
        src/table.h
        src/table.cpp
        src/services/httpclient.h
        src/services/httpclient.cpp
        src/services/baseservice.h
        src/services/baseservice.cpp
        src/services/clusterservice.h
        src/services/clusterservice.cpp
        src/services/cursorservice.h
        src/services/cursorservice.cpp
        src/services/functionservice.h
        src/services/functionservice.cpp
        src/services/namespaceservice.h
        src/services/namespaceservice.cpp
        src/services/tenantservice.h
        src/services/tenantservice.cpp
        src/services/permissionservice.h
        src/services/permissionservice.cpp
        src/services/prestoqueryservice.h
        src/services/prestoqueryservice.cpp
        src/services/sinkservice.h
        src/services/sinkservice.cpp
        src/services/sourceservice.h
        src/services/sourceservice.cpp
        src/services/tokenservice.h
        src/services/tokenservice.cpp
        src/services/topicservice.h
        src/services/topicservice.cpp
        src/widgets/baseinputwindow.h
        src/widgets/baseinputwindow.cpp
        src/widgets/mdisubwindow.h
        src/widgets/mdisubwindow.cpp
        src/widgets/basemdisubwindow.h
        src/widgets/basemdisubwindow.cpp
        src/widgets/mainwindow.h
        src/widgets/mainwindow.cpp
        src/widgets/functioninstanceswindow.h
        src/widgets/functioninstanceswindow.cpp
        src/widgets/functionswindow.h
        src/widgets/functionswindow.cpp
        src/widgets/lastcommitmessagewindow.h
        src/widgets/lastcommitmessagewindow.cpp
        src/widgets/namespacewindow.h
        src/widgets/namespacewindow.cpp
        src/widgets/newclusterwindow.h
        src/widgets/newclusterwindow.cpp
        src/widgets/newfunctionwindow.h
        src/widgets/newfunctionwindow.cpp
        src/widgets/newtopicwindow.h
        src/widgets/newtopicwindow.cpp
        src/widgets/peekmessageswindow.h
        src/widgets/peekmessageswindow.cpp
        src/widgets/permissionswindow.h
        src/widgets/permissionswindow.cpp
        src/widgets/querytopicdatawindow.h
        src/widgets/querytopicdatawindow.cpp
        src/widgets/sendmessagewindow.h
        src/widgets/sendmessagewindow.cpp
        src/widgets/sinkinputwindow.h
        src/widgets/sinkinputwindow.cpp
        src/widgets/sinkswindow.h
        src/widgets/sinkswindow.cpp
        src/widgets/sourceinputwindow.h
        src/widgets/sourceinputwindow.cpp
        src/widgets/sourceswindow.h
        src/widgets/sourceswindow.cpp
        src/widgets/subscriptionwindow.h
        src/widgets/subscriptionwindow.cpp
        src/widgets/tenantwindow.h
        src/widgets/tenantwindow.cpp
        src/widgets/tokenwindow.h
        src/widgets/tokenwindow.cpp
        src/widgets/topicoverviewwindow.h
        src/widgets/topicoverviewwindow.cpp
        src/widgets/topicstoragewindow.h
        src/widgets/topicstoragewindow.cpp
        src/widgets/topicswindow.h
        src/widgets/topicswindow.cpp
)

# Add resources
qt_add_resources(PROJECT_SOURCES resource/resources.qrc)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PDM
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET pdm APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(PDM SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(PDM
            ${PROJECT_SOURCES}
        )
    endif()
endif()

if(APPLE)

    # Enable RPATH handling
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks;@loader_path/../Frameworks")

    # Find Pulsar library
    find_library(PULSAR_LIBRARY NAMES libpulsar.dylib HINTS ${CMAKE_SOURCE_DIR}/library/pulsar/macos/lib)
    #find_library(PULSAR_LIBRARY NAMES libpulsar.dylib HINTS /opt/homebrew/lib)
    find_path(PULSAR_INCLUDE_DIR pulsar/Client.h HINTS ${CMAKE_SOURCE_DIR}/library/pulsar/macos/include)

    # Specify the macOS bundle properties
    set_target_properties(PDM PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "PDM"
        MACOSX_BUNDLE_ICON_FILE logo.icns
        BUILD_WITH_INSTALL_RPATH "${CMAKE_BUILD_WITH_INSTALL_RPATH}"
        INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
    )

    # Include the icon file in the project
    set(ICON_FILE ${CMAKE_SOURCE_DIR}/resource/icons/logo.icns)

    # Add the icon to the bundle's Resources directory
    set_source_files_properties(${ICON_FILE} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )

    #Add the icon file to the executable
    target_sources(PDM PRIVATE ${ICON_FILE})
elseif(WIN32)
    # Add the resource file for Windows
    target_sources(PDM PRIVATE ${CMAKE_SOURCE_DIR}/resource/app.rc)

    # Optional: Ensure the executable is a Windows GUI application
    set_target_properties(PDM PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(PULSAR_LIBRARY AND PULSAR_INCLUDE_DIR)
    message(STATUS "Found Pulsar include directory: ${PULSAR_INCLUDE_DIR}")
    message(STATUS "Found Pulsar library: ${PULSAR_LIBRARY}")

    target_include_directories(PDM PRIVATE ${PULSAR_INCLUDE_DIR})
    target_link_libraries(PDM PRIVATE ${PULSAR_LIBRARY})
else()
    message(FATAL_ERROR "Pulsar library not found!")
endif()

target_link_libraries(PDM PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Core5Compat
    qca-qt6)

# Check if we are on macOS (install_name_tool is macOS-specific)
if(APPLE)
    # Add a custom command to run install_name_tool after building the executable
    add_custom_command(TARGET PDM POST_BUILD
            COMMAND install_name_tool
            # Change the library path to a relative or absolute path
            -change "${CMAKE_PREFIX_PATH}/lib/qca-qt6.framework/Versions/2/qca-qt6" "@rpath/qca-qt6.framework/Versions/2/qca-qt6"
            $<TARGET_FILE:PDM>
            COMMENT "Running install_name_tool to update library paths for PDM")
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.pdm)
endif()
set_target_properties(PDM PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS PDM
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PDM)
endif()
