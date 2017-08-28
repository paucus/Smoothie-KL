Introduction
------------
This application is responsible for generating the metadata files for Kikai
Labs 3D printers. These files are identified by the k3d file extension and
contain data extracted from the model and gcode files.
This application takes advantage of post-processor features provided by most
slicers. In order to run it, you must call the kl_postproc.bat as a post-
processor script.
Although this application has been tested in Windows environments, it should be
possible to use them in any *nix environment, such as Linux or MacOSX. You must
only ensure that python 2.7, PIL ("Python Imaging Library",
http://www.pythonware.com/products/pil/) and vtk ("The Visualization Toolkit",
http://www.vtk.org/) are installed.

How to use it
-------------
In order to use the script, you must set "kl_postproc.bat" as the post-
processor script in your slicer application. Not any slicer have post-processor
features, and not all of them work well. The necessary condition that must be
met to let this application work is that it must be given the final gcode full
path. So far, the two slicers where this has been proven to work are Slic3r
(http://slic3r.org/) and Simplify3D (https://www.simplify3d.com/). Cura support
post-processor plug-ins, but it doesn't provide the final destination path to
the script, but instead it gives a temporary path.
The second condition is that the user must save the result gcode file in the
same path where the model stl file is, keeping the same name, but changing the
file extension to ".gcode". The rationale behind this is to locate the original
stl file. The original model file is necessary to extract part of the data and
generate the preview images.

Slic3r
------
In this case it is enough to set the full path to the bat file to make it work.
For example, you can set through the user interface:
C:\Program Files\KikaiLabs\SlicerPostProc\kl_postproc.bat
or through the config file:
post_process = C:\Program Files\KikaiLabs\SlicerPostProc\kl_postproc.bat
If you choose to install the postprocessor in another directory, please modify
the path accordingly.

Simplify3D
----------
In this case you must do this through the user interface. Set:
C:\Program Files\KikaiLabs\SlicerPostProc\kl_postproc.bat [output_filepath]
If you choose to install the postprocessor in another directory, please modify
the path accordingly.
