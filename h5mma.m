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
ImportHDF5::nffil = "File not found during import.";
ReadDatasetsProgress::usage = "ReadDatasetsProgress is a real number between 0 and 1 indicating the current progress of the function ReadDatasets";

Begin["`Private`"];

Install["h5mma"];

ImportHDF5[file_String, elements_:{"Datasets"}] := 
  Module[{absfile, datasets, annotations, data, dims},
    If[FileExistsQ[file],
      absfile = AbsoluteFileName[file],
      Message[ImportHDF5::nffil];
      Return[$Failed]
    ];

    Check[

    Switch[elements,
      "Datasets"|{"Datasets"},
      ReadDatasetNamesFast[absfile],

      {"Datasets", _List},
      ReadDatasets[absfile, elements[[2]]],

      "Data"|{"Data"},
      datasets = ReadDatasetNames[absfile];
      ReadDatasets[absfile, datasets],

      {"Dimensions", _List},
      ReadDatasetDimensions[absfile, elements[[2]]],

      "Dimensions"|{"Dimensions"},
      datasets = ReadDatasetNames[absfile];
      ReadDatasetDimensions[absfile, datasets],

      {"Annotations", _List},
      ReadDatasetAttributes[absfile, elements[[2]]],

      "Annotations"|{"Annotations"},
      datasets = ReadDatasetNames[absfile];
      ReadDatasetAttributes[absfile, datasets],

      {_String, _Integer},
      datasets = ReadDatasetNames[absfile];
      ImportHDF5[absfile, {elements[[1]], datasets[[elements[[2]]]]}],

      {"Datasets"|"Dimensions"|"Annotations", _String},
      data=ImportHDF5[absfile, {elements[[1]], {elements[[2]]}}];
      If[data===$Failed, $Failed, First[data]],

      "Rules"|{"Rules"},
      datasets = ReadDatasetNames[absfile];
      annotations = ImportHDF5[absfile, "Annotations"];
      data = ImportHDF5[absfile, "Data"];
      dims = ImportHDF5[absfile, "Dimensions"];
      {"Annotations"->annotations, "Data"->data, "Datasets" ->datasets, "Dimensions"->dims},

      _,
      $Failed
    ],

    Throw["Error reading HDF5 data"], {LinkObject::linkd}]

];

End[];
EndPackage[];

