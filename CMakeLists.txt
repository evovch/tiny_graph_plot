cmake_minimum_required(VERSION 3.20.0)

project(tiny_graph_plot)

if(MSVC)
	add_definitions(/MP)
endif()

set(SOURCES
	source/canvas.cpp
	source/canvas_manager.cpp
	source/glfw_callback_functions.cpp
	source/main.cpp
	source/stb_image_write_impl.cpp
	source/text_renderer.cpp
	source/user_window.cpp
)

include_directories(include)
include_directories(third_party/glfw-3.3.2.bin.WIN64/include)
include_directories(third_party/glew-2.1.0/include)
include_directories(third_party/stbi)

# Choose one depending on your build environment.
link_directories(third_party/glfw-3.3.2.bin.WIN64/lib-mingw-w64)
#link_directories(third_party/glfw-3.3.2.bin.WIN64/lib-vc2017)
#link_directories(third_party/glfw-3.3.2.bin.WIN64/lib-vc2019)

link_directories(third_party/glew-2.1.0/lib/Release/x64)

add_executable(tiny_graph_plot ${SOURCES})

target_link_libraries(tiny_graph_plot glfw3)
target_link_libraries(tiny_graph_plot glew32)
target_link_libraries(tiny_graph_plot opengl32)

install(TARGETS tiny_graph_plot DESTINATION bin)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
