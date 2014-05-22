(* Mathematica Test File *)
Print[$Version];
testH5 = FileNameJoin[{$h5mmaInstallationDirectory, "h5mma", "Tests", "test.h5"}];
shortintH5 = FileNameJoin[{$h5mmaInstallationDirectory, "h5mma", "Tests", "shortint.h5"}];
compoundH5 = FileNameJoin[{$h5mmaInstallationDirectory, "h5mma", "Tests", "compound.h5"}];
data = Partition[Range[1., 2000.], 10];

If[!FileExistsQ[testH5], Print["Cannot find test HDF5 file "<>ToString[testH5]]; Abort[]];

infdata = data;
For[j = 0, j < 200, j++,
 Which[
  EvenQ[j] && Divisible[j, 4],
    infdata[[j + 1, Mod[j, 10] + 1]] = Infinity,
  EvenQ[j],
    infdata[[j + 1, Mod[j, 10] + 1]] = -Infinity
 ];
];

nandata = data;
For[j = 0, j < 200, j++,
 If[EvenQ[j],
  nandata[[j + 1, Mod[j, 10] + 1]] = Indeterminate
 ]
];

infnandata = data;
For[j = 0, j < 200, j++,
 Which[
  EvenQ[j],
    infnandata[[j + 1, Mod[j, 10] + 1]] = Infinity,
  OddQ[j],
    infnandata[[j + 1, Mod[j, 10] + 1]] = Indeterminate
 ]
];

(****************************************************************)
(* ImportHDF5 - Datasets                                        *)
(****************************************************************)

Test[
    ImportHDF5[testH5, "Datasets"]
    ,
    {"/Test Data", "/Test Data including inf", "/Test Data including inf and nan", "/Test Data including nan"}
    ,
    TestID->"ImportHDF5 - Datasets"
]

(****************************************************************)
(* ImportHDF5 - data                                            *)
(****************************************************************)

Test[
    ImportHDF5[testH5, {"Datasets", "Test Data"}]
    ,
    data
    ,
    TestID->"ImportHDF5 - data"
]

(****************************************************************)
(* ImportHDF5 - inf                                            *)
(****************************************************************)

Test[
    ImportHDF5[testH5, {"Datasets", "Test Data including inf"}]
    ,
    infdata
    ,
    TestID->"ImportHDF5 - inf"
]

(****************************************************************)
(* ImportHDF5 - nan                                             *)
(****************************************************************)

Test[
    ImportHDF5[testH5, {"Datasets", "Test Data including nan"}]
    ,
    nandata
    ,
    TestID->"ImportHDF5 - nan"
]


(****************************************************************)
(* ImportHDF5 - inf and nan                                     *)
(****************************************************************)

Test[
    ImportHDF5[testH5, {"Datasets", "Test Data including inf and nan"}]
    ,
    infnandata
    ,
    TestID->"ImportHDF5 - inf and nan"
]

(****************************************************************)
(* ImportHDF5 - short int                                       *)
(****************************************************************)

Test[
    ImportHDF5[shortintH5, {"Datasets", "DS1"}]
    ,
    {{0, -1, -2, -3, -4, -5, -6}, {0, 0, 0, 0, 0, 0, 0}, {0, 1, 2, 3, 4, 
      5, 6}, {0, 2, 4, 6, 8, 10, 12}}
    ,
    TestID->"ImportHDF5 - short int"
]

(****************************************************************)
(* ImportHDF5 - compound                                        *)
(****************************************************************)

Test[
    ImportHDF5[compoundH5, {"Datasets", "compound dataset"}]
    ,
    {"Integer data" -> {1, 2, 3},
     "Float data" -> {43.209999084472656, 43.220001220703125, 43.22999954223633},
     "Double data" -> {12.31, 12.32, 12.33}}
    ,
    TestID->"ImportHDF5 - compound"
]

(****************************************************************)
(* ImportHDF5 - hyperslab interface                             *)
(****************************************************************)

slab[s__] := ImportHDF5[testH5, {"Datasets", {"Test Data", s}}];

Test[
    slab[1 ;; All ;; 3]
    ,
    data[[1 ;; All ;; 3]]
    ,
    TestID -> "Hyperslab1"
]

Test[
    slab[1 ;; -1 ;; 3]
    ,
    data[[1 ;; -1 ;; 3]]
    ,
    TestID -> "Hyperslab2"
]

Test[
    slab[1 ;; 7 ;; 3]
    ,
    data[[1 ;; 7 ;; 3]]
    ,
    TestID -> "Hyperslab3"
]

Test[
    slab[1 ;; -7 ;; 3]
    ,
    data[[1 ;; -7 ;; 3]]
    ,
    TestID -> "Hyperslab4"
]

Test[
    slab[1 ;; -7 ;; 3, 2 ;; 4 ;; 1]
    ,
    data[[1 ;; -7 ;; 3, 2 ;; 4 ;; 1]]
    ,
    TestID -> "Hyperslab5"
]

Test[
    slab[-7 ;; -1 ;; 3, 2 ;; 4 ;; 1]
    ,
    data[[-7 ;; -1 ;; 3, 2 ;; 4 ;; 1]]
    ,
    TestID -> "Hyperslab6"
]

Test[
    slab[-1 ;; -1 ;; 3, 2 ;; 4 ;; 1]
    ,
    data[[-1 ;; -1 ;; 3, 2 ;; 4 ;; 1]]
    ,
    TestID -> "Hyperslab7"
]

Test[
    slab[1 ;; 1 ;; 3, 2 ;; 4 ;; 1]
    ,
    data[[1 ;; 1 ;; 1, 2 ;; 4 ;; 1]]
    ,
    TestID -> "Hyperslab8"
]

Test[
    Catch[slab[-1 ;; 1 ;; 3, 2 ;; 4 ;; 1]]
    ,
    "Invalid hyperslab specification"
    ,
    TestID -> "Hyperslab9"
]

Test[
    Catch[slab[3 ;; 1 ;; 3, 2 ;; 4 ;; 1]]
    ,
    "Invalid hyperslab specification"
    ,
    TestID -> "Hyperslab10"
]

Test[
    Catch[slab[1 ;; 7 ;; -3, 2 ;; 4 ;; 1]]
    ,
    "Invalid hyperslab specification"
    ,
    TestID -> "Hyperslab11"
]

Test[
    ImportHDF5[testH5, {"Datasets", {{"Test Data", 1 ;; -2 ;; 3}}}][[1]]
    ,
    data[[1 ;; -2 ;; 3]]
    ,
    TestID -> "Hyperslab"
]
