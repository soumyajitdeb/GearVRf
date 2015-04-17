## Sample Model Loader GVRF App

This application demonstrates the functionality provided by the assimp integration of GVRF which enables it to load collada and obj 3D modelling files.

The sponza model included as asset in this app was downloaded as obj from this[http://www.crytek.com/cryengine/cryengine3/downloads] website and converted into collada model.

The second cube model was created using blender.


[Assumption]
All the 3D models which are being used to load using the loadScene(...) method, are having textures included as a part of the file. Only those 3D models can be loaded which has textures.