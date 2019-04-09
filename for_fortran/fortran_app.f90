! Example of how to use Intel SDE markers for Fortran
!
! Author: Georg Zitzlsberger (georg.zitzlsberger<ad>vsb.cz)
! Copyright (C) 2019 Georg Zitzlsberger, IT4Innovations,
!                    VSB-Technical University of Ostrava, Czech Republic
!
! This program is free software: you can redistribute it and/or modify
! it under the terms of the GNU General Public License as published by
! the Free Software Foundation, either version 3 of the License, or
! (at your option) any later version.
! This program is distributed in the hope that it will be useful,
! but WITHOUT ANY WARRANTY; without even the implied warranty of
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
! GNU General Public License for more details.
! You should have received a copy of the GNU General Public License
! along with this program.  If not, see <https://www.gnu.org/licenses/>.
!
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

