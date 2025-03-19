cmake_minimum_required(VERSION 3.10)
project(MyFileSystem C)


set(SRC_FILES
    src/fs/source/dir.c
    src/fs/source/file.c
    src/fs/source/init.c
    src/fs/source/print.c
    src/fs/source/utils.c
    src/cli/source/cli.c
    src/main.c
)


include_directories(
    src/
    src/fs/include
    src/cli/include
)


add_executable(my_fs ${SRC_FILES})

set_target_properties(my_fs PROPERTIES C_STANDARD 99)
