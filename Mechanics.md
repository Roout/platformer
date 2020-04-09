##Weapon mechanic

When any unit attacks, the projectile will be created according to the weapon used by the unit.
Projectile has a lifetime.

|  Type | Behaviour |
| :------------: | ------------ |
| arrows | <ul><li>flying along the straight line</li><li>has a long lifetime</li><li>disappear on the first contact with target</li><li>deal damage to the first contacted target</li><li>has a small rectangle shape</li></ul>|
| sword trail  |  <ul><li>appear before the attacker face</li><li>has a short lifetime</li><li>has a larger rectangle shape</li><li>disappear on the collision</li><li>deal damage to the first contacted target</li></ul> |

Sequence of the events on attack:
~~~C++
unit->attack() {
	auto projectile = weapon->create_projectile();
	auto callback_on_collision = [&projectile] (auto& collider) {
		projectile->effect(collider);
	};
	projectile_manager->add_projectile(projectile, callback_on_collision);
}
~~~
**Problem:**
Callback function need either to capture the target which will be effected by projection either to pass it as a parametr from the code where collision is handled.
**Solution:**
Discarding the first approach I have to adjust code for the second approach. The physical bodies have to know about their model. Obvious disadvantage is introduction of the new dependency.





