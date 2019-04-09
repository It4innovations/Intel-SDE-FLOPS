# Example of how to use Intel SDE markers for Python scripts
#
# Author: Georg Zitzlsberger (georg.zitzlsberger<ad>vsb.cz)
# Copyright (C) 2019 Georg Zitzlsberger, IT4Innovations,
#                    VSB-Technical University of Ostrava, Czech Republic
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
import numpy as np
from ctypes import cdll
lib_sde_markers = cdll.LoadLibrary('./libsde_markers.so')

a = np.float64(1)
b = np.float64(1)
c = np.float64(1)

x = np.float32(1)
y = np.float32(1)
z = np.float32(1)

c = a + b
z = x * y

lib_sde_markers.sde_start_marker() # only count the two following lines...
a = b + c # one double precision FLOP
x = y * z # one single precision FLOP
lib_sde_markers.sde_stop_marker()

c = a + b
z = x * y

