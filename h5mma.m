(* ::Package:: *)

(*  Copyright 2010 Ian Hinder and Barry Wardell

    This file is part of H5MMA.

    H5MMA is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kranc is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Kranc; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*)

(****************************************************************************)
(* Functions for importing data from HDF5 files.                            *)
(* Uses an optional MathLink executable written in C, if available.         *)
(****************************************************************************)

BeginPackage["h5mma`"];

ImportHDF5::usage = "ImportHDF5[\"file\"] imports data from an HDF5 file, returning a complete \!\(\*
StyleBox[\"Mathematica\",\nFontSlant->\"Italic\"]\) version of it.
ImportHDF5[\"file\", \!\(\*
StyleBox[\"elements\", \"TI\"]\)] imports the specified elements from a file.";

Begin["`Private`"];

Install["h5mma"];

ImportHDF5[file_String, elements_:{"Datasets"}] := 
  Module[{useml, datasets, annotations, data, dims},
    Switch[elements,
      "Datasets"|{"Datasets"},
      ReadDatasets[file],

      {"Datasets", _String},
      ReadDataset[file, elements[[2]]],

      "Data"|{"Data"},
      datasets = ReadDatasets[file];
      ReadDataset[file, #]& /@ datasets,

      {"Dimensions", _String},
      ReadDatasetDimensions[file, elements[[2]]],

      "Dimensions"|{"Dimensions"},
      datasets = ReadDatasets[file];
      ReadDatasetDimensions[file, #]& /@ datasets,

      {"Annotations", _String},
      ReadDatasetAttributes[file, elements[[2]]],

      "Annotations"|{"Annotations"},
      datasets = ReadDatasets[file];
      ReadDatasetAttributes[file, #]& /@ datasets,

      {_String, _Integer},
      datasets = ReadDatasets[file];
      ImportHDF5[file, {elements[[1]], datasets[[elements[[2]]]]}],

      {_String, _List},
      ImportHDF5[file, {elements[[1]], #}]& /@ elements[[2]],

      "Rules"|{"Rules"},
      datasets = ReadDatasets[file];
      annotations = ImportHDF5[file, "Annotations"];
      data = ImportHDF5[file, "Data"];
      dims = ImportHDF5[file, "Dimensions"];
      {"Annotations"->annotations, "Data"->data, "Datasets" ->datasets, "Dimensions"->dims},

      _,
      ReadDatasets[file]
    ]
];

End[];
EndPackage[];

