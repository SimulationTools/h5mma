(* ::Package:: *)

(* Copyright 2010-2011 Barry Wardell and Ian Hinder

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
*)

BeginPackage["h5mma`"];

ImportHDF5::usage = "ImportHDF5[\"file\"] imports data from an HDF5 file, returning a complete \!\(\*
StyleBox[\"Mathematica\",\nFontSlant->\"Italic\"]\) version of it.
ImportHDF5[\"file\", \!\(\*
StyleBox[\"elements\", \"TI\"]\)] imports the specified elements from a file.";
Turbo::usage = "Turbo is an option for ImportHDF5 which enables faster, but less reliable dataset listing.";
ImportHDF5::nffil = "File not found during import.";
ReadDatasetsProgress::usage = "ReadDatasetsProgress is a real number between 0 and 1 indicating the current progress of the function ReadDatasets";

$h5mmaInformation::usage = "$h5mmaInformation is a list of rules that gives information about the version of h5mma you are running.";
$h5mmaInstallationDirectory::usage = "$h5mmaInstallationDirectory gives the top-level directory in which h5mma is installed.";

$h5mmaVersionNumber::usage = "$h5mmaVersionNumber is a real number which gives the current h5mma version number.";
$h5mmaReleaseNumber::usage = "$h5mmaReleaseNumber is an integer which gives the current h5mma release number.";
$h5mmaVersion::usage = "$h5mmaVersionNumber is a string that gives the version of h5mma you are running.";

$h5mmaRemote;

Begin["`Private`"];

Module[{installed},
  If[ValueQ[$h5mmaRemote],
    Module[{host, p1, p2},
      host = $h5mmaRemote[[1]];
      {p1, p2} = ToString /@ $h5mmaRemote[[2]];
      installed = Install[p1 <> "@" <> host <> "," <> p2 <> "@" <> host, LinkMode -> Connect, LinkProtocol -> "TCPIP"];
    ]
  ,
    installed = Install["h5mma"];
  ];

  If[installed === $Failed,
    Print["h5mma has been installed but the MathLink executable cannot be loaded. Check the README file for instructions on compiling it."];
    Abort[];
  ];
];

Options[ImportHDF5] = {Turbo->False};

ImportHDF5[file_String, elements_:{"Datasets"}, OptionsPattern[]] := 
  Module[{absfile, datasets, annotations, data, dims, turbo},
    Which[
     FileExistsQ[file],
      absfile = AbsoluteFileName[file],
     ValueQ[$h5mmaRemote],
      absfile = file,
     True,
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


$h5mmaInstallationDirectory = FileNameDrop[FindFile["h5mma`"], -2];

$h5mmaVersionNumber        = 1.0;
$h5mmaReleaseNumber        = 0;

$h5mmaVersion :=
 Module[{path, version, release, buildid, gitrev},
  path = $h5mmaInstallationDirectory;
  version = ToString[NumberForm[$h5mmaVersionNumber, {Infinity, 1}]];
  release = ToString[$h5mmaReleaseNumber];

  buildid = Quiet@ReadList[FileNameJoin[{path, "BUILD_ID"}], "String"];
  If[SameQ[buildid, $Failed],
    buildid = "";
  ,
    buildid = " (" <> First[buildid] <> ")";
  ];

  gitrev = Quiet@ReadList[FileNameJoin[{path, "GIT_REVISION"}],"String"];
  If[SameQ[gitrev, $Failed],
    gitrev = Quiet@First@ReadList["!git --git-dir "<>FileNameJoin[{path, ".git"}]<>" rev-parse HEAD", String];
  ,
    gitrev = First[gitrev];
  ];

  If[!StringQ[gitrev], gitrev = "", gitrev = " (" <> gitrev <> ")"];

  version <> "." <> release <> buildid <> gitrev
]

$h5mmaInformation :=
  {"InstallationDirectory" -> $h5mmaInstallationDirectory,
   "Version" -> $h5mmaVersion,
   "VersionNumber" -> $h5mmaVersionNumber,
   "ReleaseNumber" -> $h5mmaReleaseNumber}

End[];
EndPackage[];

