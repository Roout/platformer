# Welcome to 2d platformer game  
  
For now it's just a test project. My wish is to finish the game. The final step will be publishing the game to 
the google play.

Target platforms:  
- android;
- win32.

## Roadmap  

### v0.0 Set up the project  
Consist of linking dragonbones library and setting up the compilable and linkable game project.  
- [x] add Dragon Bones  
	+ adapted sources of the current version of the [dragon bones](https://github.com/DragonBones/DragonBonesCPP/tree/dev) to new version of the [cocos2dx v4.0](https://github.com/cocos2d/cocos2d-x/tree/cocos2d-x-4.0). It may be useless though.  
	+ added cmake files for the dragon bones to compile it as static library   
	+ changed root CMakeLists file (link dragonBones library to target)  
- [x] add git  
- [x] change required cmake version to compile ( switched to `target_precompile_headers` so minimum cmake version is 3.16)  
  
### v0.1 Basic game structure  
- [x] add character: model and view as rectangle  
- [x] add simple map [256 * 16]  
- [x] implement character movement without physics  
- [x] implement own physics system  
  
### v0.2 Add weapon system!  
- [x] create weapon system  
- [x] add melee weapons  
- [x] extend unit class: add health bar and weapon holder  
- [x] add basic trap  
- [x] switched to chipmunk and update movement system  
- [x] refactoring  

### v0.3 Add enemies  
- [ ] add automatic behavior to enemies:  
	- [x] aggro (field of influence): define area in json file waypoints where the enemy is active and will attack anyone who cross it  
	- [ ] communication  
	- [ ] attacking:  
		- [x] axe   
		- [ ] bow  
		- [ ] throwing boulders  
	- [x] pursuit  
	- [x] patrol (pathfinding): define path graph in json file waypoints  
	- [ ] defence   
	- [ ] anything else? Does it actually fit the genre    
  
**v0.4** Boss  
- [ ] add boss  

---
Current step: **v0.3**




