cmake_minimum_required(VERSION 3.10)
project(MyAllocatorTest)

# Добавляем поддиректорию с аллокатором
add_subdirectory(Allocator_src)

# Добавляем поддиректорию с тестовым проектом
add_subdirectory(test_project)
