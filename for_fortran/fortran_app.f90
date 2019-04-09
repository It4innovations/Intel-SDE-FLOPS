program fortran_app
    implicit none
    real(kind=8) a, b, c
    real(kind=4) x, y, z

    interface
        subroutine sde_start_marker() bind(C, name="sde_start_marker")
        end subroutine sde_start_marker
        subroutine sde_stop_marker() bind(C, name="sde_stop_marker")
        end subroutine sde_stop_marker
    end interface

    c = a + b
    z = x * y

    call sde_start_marker() ! only count the two following lines...
    a = b + c ! one double precision FLOP
    x = y * z ! one single precision FLOP
    call sde_stop_marker()

    c = a + b
    z = x * y
end program fortran_app

