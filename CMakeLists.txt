cmake_minimum_required(VERSION 3.5)
project(fb_anim)

add_executable(fb_anim main.c)

# Install the binary to /usr/bin
install(TARGETS fb_anim DESTINATION bin)
