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

h5mmaMathLink::usage = "h5mmaMathLink specifies whether ImportHDF5 should use a MathLink executable when importing.";

Begin["`Private`"];

$h5mmaLink = Install["h5mma"];

Options[ImportHDF5] = { h5mmaMathLink -> If[$h5mmaLink=!=$Failed, True, False] };
ImportHDF5[file_String, elements_:{"Datasets"}, opts:OptionsPattern[]] := 
  Module[{useml, datasets, annotations, data, dims},
    useml = OptionValue[h5mmaMathLink];
    
    (* If the MathLink file is not found, then just use Mathematica's built-in HDF5 support *)
    If[!useml, Return[Import[file, elements, Evaluate[FilterRules[{opts}, Options[Import]]]]]];

    Switch[elements,
      "Datasets"|{"Datasets"},
      ReadDatasets[file],

      {"Datasets", _String},
      ReadDataset[file, elements[[2]]],

      {"Datasets", _List},
      ReadDataset[file, #]& /@ elements[[2]],

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

