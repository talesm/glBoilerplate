# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
#
# Author: [talesm](https://github.com/talesm)
#.rst:
# FindSDL2
# -------
#
# Looks for SDL and its most used satellite libraries.
#
# By default it exports only SDL itself, but you can require additional libraries as
#  components. To do this, use the library name __without__ the `SDL2_` prefix. E.g:
#
# ```
# find_package(SDL2 REQUIRED MODULE COMPONENTS gfx image main)
# ```
#
# The tested libraries are:
#
# - SDL2_gfx
# - SDL2_image
# - SDL2_ttf
# - SDL2_mixer
# - SDL2_main (Not a separated library, but as some projects don't need it, it was made optional)
#
# And it defines the following variables:
#
# * SDL2_FOUND: ON if SDL and all required components are found, OFF otherwise
# * SDL2_INCLUDE_DIRS: The SDL2 include directory and all the components include dirs
# * SDL2_LIBRARIES: The SDL2 library and all its components libraries
# * SDL2_INCLUDE_DIR: The SDL2 include directory and nothing more
# * SDL2_LIBRARY: The SDL2 library and nothing more
# * SDL2_<C>_INCLUDE_DIR: The SDL2_<C> include directory where c is one of the compoenent names above
# * SDL2_<C>_LIBRARY: The SDL2_<C> library where c is one of the compoenent names above
find_library(SDL2_LIBRARY SDL2)
find_path(SDL2_INCLUDE_DIR SDL.h PATH_SUFFIXES SDL2)

if((${SDL2_LIBRARY} STREQUAL "SDL2_LIBRARY-NOTFOUND") OR (${SDL2_INCLUDE_DIR} STREQUAL "SDL2_INCLUDE_DIR-NOTFOUND") )
  set(SDL2_FOUND OFF)
else()
  set(SDL2_FOUND ON)
  set(SDL2_LIBRARIES ${SDL2_LIBRARY})
  set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})

  foreach(_SDL_COMPONENT ${SDL2_FIND_COMPONENTS})
    if(${_SDL_COMPONENT} STREQUAL "main")
      find_library(SDL2_${_SDL_COMPONENT}_LIBRARY SDL2${_SDL_COMPONENT})
      if(${SDL2_${_SDL_COMPONENT}_LIBRARY} STREQUAL "SDL2_${_SDL_COMPONENT}_LIBRARY-NOTFOUND")
        set(SDL2_${_SDL_COMPONENT}_FOUND OFF)
        if(${SDL2_FIND_REQUIRED_${_SDL_COMPONENT}})
          set(SDL2_FOUND OFF)
        endif()
      else()
        set(SDL2_${_SDL_COMPONENT}_FOUND ON)
        set(SDL2_LIBRARIES ${SDL2_LIBRARIES} ${SDL2_${_SDL_COMPONENT}_LIBRARY})
      endif()
    else()
      find_library(SDL2_${_SDL_COMPONENT}_LIBRARY SDL2_${_SDL_COMPONENT})

      if(${_SDL_COMPONENT} STREQUAL gfx)
        find_path(SDL2_${_SDL_COMPONENT}_INCLUDE_DIR SDL2_gfxPrimitives.h PATH_SUFFIXES SDL2)
      elseif(${_SDL_COMPONENT} STREQUAL main)
        set(SDL2_${_SDL_COMPONENT}_INCLUDE_DIR "")
      else()
        find_path(SDL2_${_SDL_COMPONENT}_INCLUDE_DIR SDL_${_SDL_COMPONENT}.h PATH_SUFFIXES SDL2)
      endif()

      if((${SDL2_${_SDL_COMPONENT}_LIBRARY} STREQUAL "SDL2_${_SDL_COMPONENT}_LIBRARY-NOTFOUND")
        OR (${SDL2_${_SDL_COMPONENT}_INCLUDE_DIR} STREQUAL "SDL2_${_SDL_COMPONENT}_INCLUDE_DIR-NOTFOUND")
      )
        set(SDL2_${_SDL_COMPONENT}_FOUND OFF)
        if(${SDL2_FIND_REQUIRED_${_SDL_COMPONENT}})
          set(SDL2_FOUND OFF)
        endif()
      else()
        set(SDL2_${_SDL_COMPONENT}_FOUND ON)
        set(SDL2_LIBRARIES ${SDL2_LIBRARIES} ${SDL2_${_SDL_COMPONENT}_LIBRARY})
        set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS} ${SDL2_${_SDL_COMPONENT}_INCLUDE_DIR})
      endif()
    endif()
  endforeach()
endif()

if(SDL2_FIND_REQUIRED AND (NOT ${SDL2_FOUND}))
  message(FATAL_ERROR "SDL or one of its sub libraries were not found")
endif()