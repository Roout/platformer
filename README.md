# Platformer 2D
  
This is an attempt to make a 2D Platformer game and complete the whole development cycle.  
The final step will be publishing the game to the google play.

Target platforms:

- Android
- Linux
- Windows

Now works only on windows.

## Tools

1. [Dragon Bones](https://github.com/Roout/adp-dragon-bones)
2. [Cocos2dx v4.0 engine](https://github.com/cocos2d/cocos2d-x) + tools needed by this engine (Python, etc)
3. CMake 3.16 or higher
4. MSVC compiler with c++17 or higher
5. [Tiled 1.2](https://www.mapeditor.org/)
6. [Texture Packer](https://www.codeandweb.com/texturepacker)

## Progress

You can see the current progress at videos below.  
FPS is not steady due to potato laptop: it doesn't like recording...  

### Ability showcast

Current player's abilities:

| Key | Description |
|-----|-------------|
| A, D | Move left-right |
| W, space | Jump and double jump |
| F | Sword attack. Has 3 types of animations: top, mid, bottom |
| E | Special attack, has cast time, deals more damage |
| G | Fireball attack |
| Q | Dash |

Images:  
<img src="/Resources/screenshots/walkthrough-1.PNG" alt="walkthrough" width="60%" height:="60%"/>
<img src="/Resources/screenshots/walkthrough-2.PNG" alt="walkthrough" width="60%" height:="60%"/>
<img src="/Resources/screenshots/walkthrough-3.PNG" alt="walkthrough" width="60%" height:="60%"/>
<img src="/Resources/screenshots/walkthrough-4.PNG" alt="walkthrough" width="60%" height:="60%"/>

You can see more here: [Youtube link](https://youtu.be/KvYxr2vZ7zo)

### Level walkthough

You can perceive level as a sandbox. Level boarders are shown on the debug screen.  
Now sandbox contains only boxes and typical platforms. Can be restarted and paused.  
Debug mode has several usefull flags:

- show physics level boundaries
- invicibility
- current state of the player and NPCs

Images:  
<img src="/Resources/screenshots/debug.PNG" alt="debug" width="60%" height:="60%"/>

You can see more here: [Youtube link](https://youtu.be/IM8e4cO3_FY)

### Boss fight

First boss is a forest bandit. Boss has following abitilies:

| Ability | Description |
|---------|-------------|
| Chain's attack | Slow attack ahead of yourself: doesn't move and attack 3 times, each attack longer than previous |
| Fireballs | Send several fireballs |
| Jump with chain attack | Deal damage on attack |
| Dash | Special attack, has cast time, deals more damage |
| Summon fire cloud | Calls fire cloud which attack with fireballs from the sky and doesn't depend on boss |

Images:  
<img src="/Resources/screenshots/boss.PNG" alt="boss" width="60%" height:="60%"/>

You can see more here: [Youtube link](https://youtu.be/i9K5rqW_JoM)

## Credits

[Andrew Sadovnikov](https://itch.io/profile/andrwood) - graphics/animations  
[Sergei Nevstruev](https://github.com/Roout) - programming
