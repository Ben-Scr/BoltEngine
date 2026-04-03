#pragma once

namespace Bolt {
	struct IdTag {};
	struct StaticTag {};
	struct DisabledTag {};
	struct DeadlyTag{};


	//template<typename... T>
	//using Exclude = entt::exclude_t<T...>;
	//using ActiveEntities = Exclude<DisabledTag>;
}