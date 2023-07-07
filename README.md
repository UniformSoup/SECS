# SECS
__Pronounced '[ˈsɛks]'__

A Small Entity Component System written in C++17.

It needs C++17 because it uses a good few fold expressions.

Simple Example:
```cpp
SECS::EntityManager<int, char, SomeComponent> manager;
SECS::Entity                                  anEntity = manager.create();
...

SomeComponent& component = manager.add<SomeComponent>(anEntity);
manager.add<char>(anEntity);
manager.remove<SomeComponent>(anEntity);
char& c = manager.get<char>(anEntity);
...

\\ Print all Entities:
for (auto& i : manager.entities())
  std::cout << i << ' ';

...

\\ Print all Entities with SomeComponent:
for (auto& i : manager.entities<SomeComponent>())
  std::cout << i << ' ';

...

\\ Print all Entities with SomeComponent and a char:
for (auto& i : manager.entities<SomeComponent, char>())
  std::cout << i << ' ';

...

```
