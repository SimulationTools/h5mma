(* Copyright (C) 2010 Ian Hinder and Barry Wardell *)

BeginPackage["h5mma`"];

ImportHDF5::usage = "ImportHDF5[\"file\"] imports data from an HDF5 file, returning a complete \!\(\*
StyleBox[\"Mathematica\",\nFontSlant->\"Italic\"]\) version of it.
ImportHDF5[\"file\", \!\(\*
StyleBox[\"elements\", \"TI\"]\)] imports the specified elements from a file.";
Turbo::usage = "Turbo is an option for ImportHDF5 which enables faster, but less reliable dataset listing.";
ImportHDF5::nffil = "File not found during import.";
ReadDatasetsProgress::usage = "ReadDatasetsProgress is a real number between 0 and 1 indicating the current progress of the function ReadDatasets";

Begin["`Private`"];

Install["h5mma"];

Options[ImportHDF5] = {Turbo->False};

ImportHDF5[file_String, elements_:{"Datasets"}, OptionsPattern[]] := 
  Module[{absfile, datasets, annotations, data, dims, turbo},
    If[FileExistsQ[file],
      absfile = AbsoluteFileName[file],
      Message[ImportHDF5::nffil];
      Return[$Failed]
    ];

    turbo = OptionValue[Turbo];

    Check[

    Switch[elements,
      "Datasets"|{"Datasets"},
      If[turbo, ReadDatasetNamesFast[absfile], ReadDatasetNames[absfile]],

      {"Datasets", _List},
      ReadDatasets[absfile, elements[[2]]],

      "Data"|{"Data"},
      datasets = ImportHDF5[absfile];
      ReadDatasets[absfile, datasets],

      {"Dimensions", _List},
      ReadDatasetDimensions[absfile, elements[[2]]],

      "Dimensions"|{"Dimensions"},
      datasets = ImportHDF5[absfile];
      ReadDatasetDimensions[absfile, datasets],

      {"Annotations", _List},
      ReadDatasetAttributes[absfile, elements[[2]]],

      "Annotations"|{"Annotations"},
      datasets = ImportHDF5[absfile];
      ReadDatasetAttributes[absfile, datasets],

      {_String, _Integer},
      datasets = ImportHDF5[absfile];
      ImportHDF5[absfile, {elements[[1]], datasets[[elements[[2]]]]}],

      {"Datasets"|"Dimensions"|"Annotations", _String},
      data=ImportHDF5[absfile, {elements[[1]], {elements[[2]]}}];
      If[data===$Failed, $Failed, First[data]],

      "Rules"|{"Rules"},
      datasets = ImportHDF5[absfile];
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

