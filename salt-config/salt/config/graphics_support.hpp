#pragma once

#ifdef SALT_TARGET_OPENGL
#    if SALT_TARGET(APPLE)
#        define SALT_OPENGL_VERSION_MAJOR (4)
#        define SALT_OPENGL_VERSION_MINOR (1)
#        define SALT_GLSL_VERSION         "#version 410"
#    else
#        define SALT_OPENGL_VERSION_MAJOR (4)
#        define SALT_OPENGL_VERSION_MINOR (6)
#        define SALT_GLSL_VERSION         "#version 460"
#    endif
#endif