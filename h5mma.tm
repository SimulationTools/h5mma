:Evaluate: BeginPackage["h5mma`"]
:Evaluate: h5mma::mlink = "Error from MathLink executable: `1`."

:Evaluate: Begin["`Private`"]

:Begin: 
:Function:       ReadDatasets
:Pattern:        ReadDatasets[fileName_String, datasets:{__String}, slabs:{{___Span}..}]
:Arguments:      { fileName, datasets, slabs }
:ArgumentTypes:  { String, Manual }
:ReturnType:     Manual
:End:

:Begin: 
:Function:       ReadDatasetNames
:Pattern:        ReadDatasetNames[fileName_String]
:Arguments:      { fileName }
:ArgumentTypes:  { String }
:ReturnType:     Manual
:End:

:Begin:
:Function:       ReadDatasetNamesFast
:Pattern:        ReadDatasetNamesFast[fileName_String]
:Arguments:      { fileName }
:ArgumentTypes:  { String }
:ReturnType:     Manual
:End:

:Begin:
:Function:       ReadDatasetDimensions
:Pattern:        ReadDatasetDimensions[fileName_String, datasets:{__String}]
:Arguments:      { fileName, datasets }
:ArgumentTypes:  { String, Manual }
:ReturnType:     Manual
:End:

:Begin: 
:Function:       ReadDatasetAttributes
:Pattern:        ReadDatasetAttributes[fileName_String, datasets:{__String}]
:Arguments:      { fileName, datasets }
:ArgumentTypes:  { String, Manual }
:ReturnType:     Manual
:End: 

:Evaluate: End[]
:Evaluate: EndPackage[]
