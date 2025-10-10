cmake_minimum_required(VERSION 3.16)
project(BNTCandles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.9.2/mingw_64" CACHE PATH "Qt6 Library Path")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Gui Sql Widgets Charts REQUIRED)

find_library(MYSQL_LIBRARY
    NAMES libmysql mysqlclient
    PATHS
        "C:/Program Files/MySQL/MySQL Connector C 8.0/lib"
        "C:/Program Files/MySQL/MySQL Connector C 6.1/lib"
        "C:/Program Files/MySQL/MySQL Server 8.0/lib"
)
if(NOT MYSQL_LIBRARY)
    message(FATAL_ERROR "MySQL Connector/C library (libmysql.lib) not found. Please install MySQL Connector/C or update the path.")
endif()

find_path(MYSQL_INCLUDE_DIR
    NAMES mysql.h
    PATHS
        "C:/Program Files/MySQL/MySQL Connector C 8.0/include"
        "C:/Program Files/MySQL/MySQL Connector C 6.1/include"
        "C:/Program Files/MySQL/MySQL Server 8.0/include"
)
if(NOT MYSQL_INCLUDE_DIR)
    message(FATAL_ERROR "MySQL Connector/C headers (mysql.h) not found. Please install MySQL Connector/C or update the path.")
endif()

set(SOURCES
    main.cpp
    mainwindow.cpp

    addcustomer.cpp

    producteditor.cpp
    addproductdetailswindow.cpp

    ordereditor.cpp
    addorderdetailwindow.cpp
    
    expenseeditor.cpp
    addExpenseDetailWindow.cpp

    reportspage.cpp
    
    CustomerModel.cpp
)

set(HEADERS
    mainwindow.h

    addcustomer.h

    producteditor.h
    addproductdetailswindow.h

    ordereditor.h
    addorderdetailwindow.h

    expenseeditor.h
    addExpenseDetailWindow.h

    reportspage.h

    CustomerModel.h
)

set(FORMS
    mainwindow.ui

    addcustomer.ui

    producteditor.ui
    addproductdetailswindow.ui

    ordereditor.ui
    addorderdetailwindow.ui

    expenseeditor.ui
    addExpenseDetailWindow.ui

    reportspage.ui
 
    
)

set(RESOURCES
    resources.qrc
    
    db_cred.ini
    taxRates.txt
    states
    bnt_icon.jpg
)

add_executable(BNTCandles 
    ${SOURCES} 
    ${HEADERS} 
    ${FORMS} 
    ${RESOURCES}
    


)

target_link_libraries(BNTCandles PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Sql
    Qt6::Widgets
    Qt6::Charts
    ${MYSQL_LIBRARY}
)

target_include_directories(BNTCandles PRIVATE
    ${MYSQL_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Debug UIC processing
message(STATUS "UI files: ${FORMS}")
