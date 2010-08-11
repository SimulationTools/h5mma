:Evaluate: BeginPackage["h5mma`"]
:Evaluate: Begin["`Private`"]

:Begin: 
:Function:       ReadDatasets
:Pattern:        ReadDatasets[fileName_String]
:Arguments:      { fileName } 
:ArgumentTypes:  { String } 
:ReturnType:     Manual
:End: 

:Begin: 
:Function:       ReadDataset
:Pattern:        ReadDataset[fileName_String, datasetName_String]
:Arguments:      { fileName, datasetName } 
:ArgumentTypes:  { String, String } 
:ReturnType:     Manual
:End: 

:Begin: 
:Function:       ReadDatasetAttributes
:Pattern:        ReadDatasetAttributes[fileName_String, datasetName_String]
:Arguments:      { fileName, datasetName } 
:ArgumentTypes:  { String, String } 
:ReturnType:     Manual
:End: 

:Evaluate: End[]
:Evaluate: EndPackage[]
