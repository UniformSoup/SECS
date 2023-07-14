#pragma once

#include <bitset>
#include <unordered_map>
#include <unordered_set>

// Thank god ChatGPT understands variadic templates and fold expressions.

// Small Entity Component System
namespace SECS
{
	using Entity = unsigned int;

	template <typename Component>
	using EntityMap = std::unordered_map<Entity, Component>;

	template <typename... Components>
	struct ComponentTypes : public std::bitset<sizeof...(Components)>
	{
			template <typename Component>
			static constexpr std::size_t index()
			{
				std::size_t idx	  = 0;
				bool		found = false;
				(((found || std::is_same_v<Component, Components>) ? found = true : ++idx), ...);
				return idx;
			}

			template <typename Component>
			bool const& get() const
			{
				return this->operator[](index<Component>());
			}

			template <typename Component>
			void set(bool const& bit)
			{
				this->operator[](index<Component>()) = bit;
			}

			template <typename... ComponentList>
			static constexpr ComponentTypes make()
			{
				ComponentTypes types;
				((types.template set<ComponentList>(true)), ...);
				return types;
			}

			template <typename... ComponentList>
			bool has() const
			{
				return ComponentTypes().template make<ComponentList...>() == (*this & ComponentTypes().template make<ComponentList...>());
			}
	};

	template <typename... Components>
	class EntityManager
	{
			using Types		   = ComponentTypes<Components...>;
			using ComponentMap = EntityMap<Types>;

			std::tuple<ComponentMap, EntityMap<Components>...> registry;
			std::unordered_set<Entity>						   alive, dead;

		public:

			Entity create()
			{
				static Entity count = 0;
				Entity		  id;

				if (!dead.empty())
				{
					id = *dead.begin();
					dead.erase(dead.begin());
				}
				else
					id = count++;

				std::get<ComponentMap>(registry)[id] = {};
				alive.insert(id);
				return id;
			}

			void destroy(Entity const& id)
			{
				std::get<ComponentMap>(registry).erase(id);
				(std::get<EntityMap<Components>>(registry).erase(id), ...);
				alive.erase(id);
				dead.insert(id);
			}

			template <typename... ComponentList>
			class Iterator
			{
					std::unordered_set<Entity>::const_iterator current;
					std::unordered_set<Entity>::const_iterator end;
					ComponentMap const&						   types;

					bool valid(Entity const& e)
					{
						return (!(Types().template make<ComponentList...>().any()) || types.at(e).template has<ComponentList...>());
					}

					void advance()
					{
						while (current != end && !valid(*current)) ++current;
					}

				public:

					Iterator() = default;

					Iterator(std::unordered_set<Entity>::const_iterator const& begin,
							 std::unordered_set<Entity>::const_iterator const& end,
							 ComponentMap const&							   types)
						: current(begin), end(end), types(types)
					{
						advance();
					}

					Entity const& operator*() const { return *current; }

					Entity const* operator->() { return current.operator->(); }

					Iterator& operator++()
					{
						++current;
						advance();
						return *this;
					}

					Iterator operator++(int)
					{
						Iterator tmp = *this;
						++(*this);
						return tmp;
					}

					friend bool operator==(Iterator const& a, Iterator const& b) { return a.current == b.current; }

					friend bool operator!=(Iterator const& a, Iterator const& b) { return a.current != b.current; }
			};

			template <typename... ComponentList>
			struct IteratorRange
			{
					std::unordered_set<Entity> const& alive;
					ComponentMap const&				  types;

					Iterator<ComponentList...> begin() const
					{
						return Iterator<ComponentList...>(alive.begin(), alive.end(), types);
					}

					Iterator<ComponentList...> end() const
					{
						return Iterator<ComponentList...>(alive.end(), alive.end(), types);
					}

					IteratorRange(std::unordered_set<Entity> const& alive, ComponentMap const& types)
						: alive(alive), types(types) {}
			};

			template <typename... ComponentList>
			IteratorRange<ComponentList...> entities() const
			{
				return { alive, std::get<ComponentMap>(registry) };
			}

			template <typename Component>
			Component& get(Entity const& entity)
			{
				return std::get<EntityMap<Component>>(registry).at(entity);
			}

			template <typename Component>
			Component& add(Entity const& entity)
			{
				std::get<ComponentMap>(registry)[entity].template set<Component>(true);
				return std::get<EntityMap<Component>>(registry)[entity];
			}

			template <typename Component>
			void remove(Entity const& entity)
			{
				std::get<ComponentMap>(registry)[entity].template set<Component>(false);
				std::get<EntityMap<Component>>(registry).erase(entity);
			}

			template <typename Component>
			std::vector<Component*> components()
			{
				std::vector<Component*> c;

				for (auto& pair : std::get<EntityMap<Component>>(registry))
					c.push_back(&(pair.second));

				return c;
			}

			template <typename... ComponentList>
			bool has(Entity const& entity)
			{
				return std::get<ComponentMap>(registry)[entity].template has<ComponentList...>();
			}

			bool isAlive(Entity const& entity) { return alive.count(entity); }
	};
}