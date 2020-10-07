# Notes

`CONTENT SIZE` - size of image (scaled)
`PHYSICS BODY SIZE` - size of collidable part of any entity
`HIT BOX SIZE` - size of box that responsible for taking damage, i.e. part that on intersection with projectile or trap decrease the health points  

A weapon create a projectile on the border of the `CONTENT SIZE` so an AI { Axe Warrior, Spearman } decide whether the target (player) can be attacked relying on own `CONTENT SIZE` and player's `HIT BOX SIZE`. Pursuit mode of the Warrior & Spearman also relying on these variables.