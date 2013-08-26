(* Mathematica Test File *)
testH5 = FileNameJoin[{DirectoryName[FindFile["h5mma`"]], "Tests", "test.h5"}];
Print[testH5];
data = Partition[Range[1., 2000.], 10];

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
