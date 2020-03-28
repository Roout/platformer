# Welcome to 2d platformer game  
---
For now it's just a test project. My with is to complete it. The final step will be publishing the game to 
the google play.

Target platforms:  
- android;
- win32.

## Roadmap  

**v0.0** _Set up the project_  
Consist of linking dragonbones library and setting up the compilable and linkable game project.  
+ add Dragon Bones
	+ adapted sources of the current version of the [dragon bones](https://github.com/DragonBones/DragonBonesCPP/tree/dev) to new version of the [cocos2dx v4.0](https://github.com/cocos2d/cocos2d-x/tree/cocos2d-x-4.0). It may be useless though.
	+ added cmake files for the dragon bones to compile it as static library   
	+ changed root CMakeLists file (link dragonBones library to target)
+ add git
+ change required cmake version to compile ( switched to `target_precompile_headers` so minimum cmake version is 3.16)

**v0.1** _Basic game structure_  
- add character: model and view as rectangle 
- add simple map [20 * 20]  
- implement character movement without physiscs
- move/jump - maybe use chipmunk/box2d 

**v0.2** _Build game level_
- extend map: landscape, platforms, minor obstacles, traps

**v0.3** _Enemies_
- add static enemies
- add weapons to enemies and hero
- add automatic behavior to enemies: 
	- aggro
	- communication
	- targeting // bow?
	- pursuit
	- pasfinding
	- defence
	- anything else? Does it actually fit the genre

**v0.4** _Boss_
- add boss

---
Current step: **v0.0**




