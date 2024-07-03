MetaCsoundProject
===========================================

MetaCsound is an upcomming plugin for [Unreal Engine](https://www.unrealengine.com/) that aims to integrate the
audio signal processing engine [Csound](https://csound.com/) within the
[MetaSounds](https://dev.epicgames.com/documentation/en-us/unreal-engine/metasounds-in-unreal-engine) environment.
This integration will result in a new set of available nodes for MetaSounds. When executed, they will create a new Csound
instance, compile the input Csound file, and start processing the resulting audio.

This repository is a small Unreal project that includes the integrated plugin. It is a work in progress and is
not intended to be a final product. This repository serves as a way to showcase my work to others. The final
plugin will be published in its own repository without any attached Unreal project.

Using
----------------------------------------------

Using MetaCsound is very easy. In your MetaSounds graph, simply add one of the available Csound nodes.
A filepath to the `.csd` file needs to be provided. This Csound file will be played within the graph.
Optional input parameters include audio, a control-rate channel, and Csound events described
by a string and a MetaSounds event.

Please by wary that for now, all paths used are full paths, that will probably work on the machine of the author.
In order to use it in your own machine, change the paths int the `MetaCsound.Build.cs` for linking the library
correctly, as well as the path to the Csound file present in the `MSS_Wind` graph.

Please note that for now, all paths used are full paths that will likely only work on the author's machine.
To use this project on your own machine, you will need to change the paths in the `MetaCsound.Build.cs` file
to correctly link the library, as well as the path to the Csound file present in the `MSS_Wind` graph.
 
License
----------------------------------------------

	Copyright (C) 2024 Albert Madrenys Planas

	This software is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This software is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this software; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
