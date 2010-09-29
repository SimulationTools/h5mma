:Evaluate: BeginPackage["h5mma`"]
:Evaluate: Begin["`Private`"]

:Begin: 
:Function:       ReadDatasets
:Pattern:        ReadDatasets[fileName_String, datasets:{__String}]
:Arguments:      { fileName, datasets }
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
