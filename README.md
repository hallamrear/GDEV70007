# GDEV70007
Game Master Project

# Associated Dissertation 
https://staffsuniversity-my.sharepoint.com/:w:/g/personal/r017093h_student_staffs_ac_uk/IQCybp_XUgkZTLRblix2lb40AaHdp-CG6JAHCGrEDG7gCBU?e=mche8Q

# Reference List

## Books
- Game Engine Physics Development by Ian Millington
- Real-time Collision Detection by Christer Ericson

## Writeups I'm coming across.
https://realtimecollisiondetection.net/blog/?p=12#:~:text=The%20triangle%20soup%20is%20great%20in%20the,and%20apply%20collsion%20materials%20to%20the%20triangles.
https://realtimecollisiondetection.net/blog/?p=20
https://iquilezles.org/articles/

# Development Notes

## Rendering
- Using IMGUI for all the world editting business. I need to iterate FAST.
- Vulkan vs. DirectX 12
	- I'd love to use Vulkan but I have no idea if anywhere has the SDK installed.
- Forward Renderer
- Simple Lighting and Texturing
	- Simple Texturing is probably not needed too.
	- No need for PBR or Normal mapping etc.
- Simple Vertex Layouts
	- Position, Normal, UV
- A ton of debug tools.
	- Debug Lines
	- Debug Spheres
	- Debug Text Log

## Saving and Loading
- I'd like to save and reload the demo level at some point.
- This should probably be done through the IMGUI system with a check to ensure I don't overwrite things.
- I also want to store collision geometry/hierarchy data seperating from all the rendering geometry so it'll need to be saved seperately.

## Scripting
I think there is potential for an additonal scripting 'thing' by compiling scripts in a seperate DLL which is then loaded in. This could be used to setup AI behaviours or things such as doors. This is probably out of scope.

## World
- Entity based rather than Scenegraph based - Closer to the Source Engine than Unity.
	- This would mean things like trigger brushes would need to exist in the world as specific entities with no mesh/model.

## Collision Algorithm
- SAT
	- Need to lock down how to get multiple contact points at once.

## Optimisation
- Octree (More likely)
	- Large objects overlapping multiple cells are placed in both.
- BSP Tree (Less likely or only for specific things)

## BVH (Bounding Volume Hierarchy)
- Encompassing Spheres
- AABB
- Sub-tree OBB or Spheres (potentially split polygons using quadtree)
- kDot Mesh / Per-Triangle check

## The Demo

- Physics Gun

- Rube Goldberg Machine
	- Barrels falling through a series of pegs
	- Hitting a box which seesaws and throws a smaller box away.
	- Smaller box hits a ball which rolls.
	- Ball rolls into a supporting beam which drops rocks.

- Stacking Exploding Barrels

- Breakable Meshes
	- Potentially out of scope.

- Floating Simulation
	- Potentially out of scope but I don't think it'll be that hard to implement if the forces are setup correctly.




# References

64px Textures/Tilesheet
https://thatguynm.itch.io/pixelated-textures