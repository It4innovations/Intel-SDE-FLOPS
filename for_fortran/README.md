# Intel SDE Markers in Fortran
To use the markers for Intel SDE within any Fortran application, create the `libsde_markers.a` static library first. E.g.:  
  
`$ gcc sde_markers.c -fPIC -c && ar rcs libsde_markers.a sde_markers.o`  
  
Use this library within your Fortran application by adding the following interface block and calling the subroutines to place a start or stop marker:

    interface
            subroutine sde_start_marker() bind(C, name="sde_start_marker")
            end subroutine sde_start_marker
            subroutine sde_stop_marker() bind(C, name="sde_stop_marker")
            end subroutine sde_stop_marker
    end interface
    
    ...
    call sde_start_marker() ! start collecting events from here on
    ... ! 1st area of interest
    call sde_stop_marker() ! pause collection of events
    
    ... ! non-interesting section
    
    call sde_start_marker() ! resume event collection
    ... ! 2nd area of interest
    call sde_stop_marker() ! stop collection (final pause)
    ...

Also, link the static library to your Fortran application via `-L. -lsde_markers`.  
  
Intel SDE can then be used like so:  
`$ sde64 -iform -mix -dyn_mask_profile -start_ssc_mark FACE:repeat -stop_ssc_mark DEAD:repeat -- ./fortran_app`  
  
The `sde_start_marker()` and `sde_stop_marker()` starts/resumes and stops/pauses the collection of events, respectively.  
  
**Example:**

    $ gcc sde_markers.c -fPIC -c && ar rcs libsde_markers.a sde_markers.o
    $ gfortran fortran_app.f90 -L. -lsde_markers -O0 -o fortran_app
    $ sde64 -iform -mix -dyn_mask_profile -start_ssc_mark FACE:repeat -stop_ssc_mark DEAD:repeat -- ./fortran_app
    $ python ../intel_sde_flops.py
    TID: 0 (OS-TID: 4740):
            Unmasked single prec. FLOPs: 1
            Masked single prec. FLOPs: 0
            Unmasked double prec. FLOPs: 1
            Masked double prec. FLOPs: 0
            Instructions executed: 360
            FMA instructions executed: 0
    =============================================
    Sum:
            Single prec. FLOPs: 1
            Double prec. FLOPs: 1
            Total instructions executed: 360
            Total FMA instructions executed: 0

