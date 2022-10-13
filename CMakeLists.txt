cmake_minimum_required(VERSION 2.6)


project(TestSkipList C CXX)

include_directories(
    Base 
    Util
    WAL
    MemTable
    Table
    db
)

set(CXX_FLAGS 
    -g 
    -Wall
    -std=c++11
)

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
#string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")


aux_source_directory(Base SRC_LIST1)            
aux_source_directory(Util SRC_LIST2)
aux_source_directory(WAL  SRC_LIST3)
aux_source_directory(MemTable  SRC_LIST4)           
aux_source_directory(Test SRC_LIST5)                
aux_source_directory(Table SRC_LIST6)
aux_source_directory(db SRC_LIST7)

# 当前目录
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
# add_executable(TestSkipList ${SRC_LIST1} ${SRC_LIST2} ${SRC_LIST3} ${SRC_LIST4} ${SRC_LIST5} ${SRC_LIST6})
# add_executable(TestMemTable ${SRC_LIST1} ${SRC_LIST2} ${SRC_LIST3} ${SRC_LIST4} ${SRC_LIST5} ${SRC_LIST6})
# add_executable(TestTable ${SRC_LIST1} ${SRC_LIST2} ${SRC_LIST3} ${SRC_LIST4} ${SRC_LIST5} ${SRC_LIST6})
# add_executable(TestFilterBlock ${SRC_LIST1} ${SRC_LIST2} ${SRC_LIST3} ${SRC_LIST4} ${SRC_LIST5} ${SRC_LIST6})
# add_executable(TestTableBuilder ${SRC_LIST1} ${SRC_LIST2} ${SRC_LIST3} ${SRC_LIST4} ${SRC_LIST5} ${SRC_LIST6})
# add_executable(TestReadTable ${SRC_LIST1} ${SRC_LIST2} ${SRC_LIST3} ${SRC_LIST4} ${SRC_LIST5} ${SRC_LIST6})
add_executable(TmpTest ${SRC_LIST1} ${SRC_LIST2} ${SRC_LIST3} ${SRC_LIST4} ${SRC_LIST5} ${SRC_LIST6} ${SRC_LIST7})
target_link_libraries(${PROJECT_NAME}  ${CMAKE_THREAD_LIBS_INIT})


