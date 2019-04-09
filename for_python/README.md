# Intel SDE Markers in Python
To use the markers for Intel SDE within any Python script, create the `libsde_markers.so` shared library first. E.g.:\
\
`$ gcc sde_markers.c -fPIC -shared -Wl,-soname,libsde_markers.so -o libsde_markers.so`\
\
Use this library within your Python script with the `ctypes.cdll` class:

    from ctypes import cdll
    lib_sde_markers = cdll.LoadLibrary('./libsde_markers.so')
    ...
    lib_sde_markers.sde_start_marker()
    ...
    lib_sde_markers.sde_stop_marker()

Intel SDE can then be used like so:\
`$ sde64 -iform -mix -dyn_mask_profile -start_ssc_mark FACE:repeat -stop_ssc_mark DEAD:repeat -- python your_script.py`\
\
The `sde_start_marker()` and `sde_stop_marker()` starts/resumes and stops/pauses the collection of events, respectively.
