project('esp32cam_lib', 'c',
        version : '1.0.0',
        default_options : ['warning_level=3', 'default_library=static'])

include_directories = include_directories('include')

sources = []
dependencies = []

cc = meson.get_compiler('c')

if get_option('platform') == 'raspberry-pi'
        sources += ['src/esp32_cam.c']
        libs = []
endif

foreach lib : libs
        l = cc.find_library(lib, dirs: [meson.current_source_dir() / 'lib/raspberry-pi'], required: false)
        if l.found()
                dependencies += declare_dependency(dependencies: l)
        else
                dependencies += declare_dependency(link_args: ['-L' + meson.current_source_dir() / 'lib/raspberry-pi', '-l' + lib])
        endif
endforeach

# dependency
a111_dep = declare_dependency(
        include_directories: include_directories,
        sources: sources,
        dependencies: dependencies,
)
meson.override_dependency(meson.project_name(), a111_dep)

# library
if get_option('library').enabled()
        a111_lib = library(
                meson.project_name(),
                include_directories: include_directories,
                sources: sources,
                dependencies: dependencies,
                install_rpath: '$ORIGIN' # Tells downstream applications to look in the binary's directory
        )
endif

