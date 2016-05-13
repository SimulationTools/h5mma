#  h5mma

*Version 1.2.1, Copyright 2011-2016 Barry Wardell and Ian Hinder*

The h5mma package provides improved support for reading HDF5 files in
Mathematica. It is significantly faster, more memory efficient and crash
resilient than the built-in HDF5 reading support.

## Getting the package

Binary versions of the h5mma package can be downloaded from the
[project webpage](http://bitbucket.org/simulationtools/h5mma).

The source is available from the project git repository:

    :::text
    git clone https://bitbucket.org/simulationtools/h5mma

## Requirements

  - Mathematica: h5mma requires Mathematica version 6 or newer. It may be possible to get it to
    work with older versions, but this is not supported.

  - HDF5: A version of the HDF5 library providing the version 1.8 API is required to build from
    source. The minimum supported version is 1.8.13. Binary versions of the package already include
    HDF5.

## Installation

The most straightforward way to install h5mma is using the
[binary packages](http://bitbucket.org/simulationtools/h5mma/downloads).

Once you have downloaded a binary package, extract it into the Mathematica Applications directory.
To find the correct location of this directory on your system, run the following command in
Mathematica:

    :::text
    FileNameJoin[{$UserBaseDirectory, "Applications"}]

## Usage

The package may be loaded into Mathematica using the command:

    :::text
    << h5mma`

The main command provided is ImportHDF5, which mimics the behavior of Import.

## Examples

For example usage, see the h5mma.nb Mathematica notebook in the doc directory.

## Compiling

It should normally not be necessary to compile h5mma yourself. If you find you do need to do so,
instructions can be found on the [project wiki](http://bitbucket.org/simulationtools/h5mma/wiki).

## Known problems

Known bugs are recorded in the project
[bug tracker](http://bitbucket.org/simulationtools/h5mma/issues).

## License

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation; either version 2.1 of the License, or (at your option)
any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this library; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

Binary releases include the HDF5 library. Copyright information for this
library can be found in the file COPYING.HDF5.

## Authors

  - Barry Wardell
  - Ian Hinder

Max Planck Institute for Gravitational Physics / Albert Einstein Institute
Am Muehlenberg 1
14476 Potsdam
Germany

[http://www.aei.mpg.de](http://www.aei.mpg.de)