# Intel SDE Markers in Python
To use the markers for Intel SDE within any Python script, create the `libsde_markers.so` shared library first. E.g.:  
  
`$ gcc sde_markers.c -fPIC -shared -Wl,-soname,libsde_markers.so -o libsde_markers.so`  
  
Use this library within your Python script with the `ctypes.cdll` class:

    from ctypes import cdll
    lib_sde_markers = cdll.LoadLibrary('./libsde_markers.so')
    
    ...
    lib_sde_markers.sde_start_marker() # start collecting events from here on
    ... # 1st area of interest
    lib_sde_markers.sde_stop_marker() # pause collection of events
    
    ... # non-interesting section
    
    lib_sde_markers.sde_start_marker() # resume event collection
    ... # 2nd area of interest
    lib_sde_markers.sde_stop_marker() # stop collection (final pause)
    ...

Intel SDE can then be used like so:  
`$ sde64 -iform -mix -dyn_mask_profile -start_ssc_mark FACE:repeat -stop_ssc_mark DEAD:repeat -- python your_script.py`  
  
The `sde_start_marker()` and `sde_stop_marker()` starts/resumes and stops/pauses the collection of events, respectively.

**Example:**

    $ gcc sde_markers.c -fPIC -shared -Wl,-soname,libsde_markers.so -o libsde_markers.so
    $ sde64 -iform -mix -dyn_mask_profile -start_ssc_mark FACE:repeat -stop_ssc_mark DEAD:repeat -- python python_script.py
    $ python ../intel_sde_flops.py
    TID: 0 (OS-TID: 7475):
            Unmasked single prec. FLOPs: 1
            Masked single prec. FLOPs: 0
            Unmasked double prec. FLOPs: 1
            Masked double prec. FLOPs: 0
            Instructions executed: 15856
            FMA instructions executed: 0
    =============================================
    Sum:
            Single prec. FLOPs: 1
            Double prec. FLOPs: 1
            Total instructions executed: 15856
            Total FMA instructions executed: 0

