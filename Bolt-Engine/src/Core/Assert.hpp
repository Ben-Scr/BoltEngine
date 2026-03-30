#pragma once
#include <Debugging/Exceptions.hpp>

#define BT_CONTRACT_FAIL(kind, cond, ...) \
	do { \
		if (!(cond)) { \
			::Bolt::ReportContractViolation((kind), #cond, ::Bolt::ContractMessage(__VA_ARGS__), std::source_location::current()); \
		} \
	} while (0)


#ifdef BT_PLATFORM_WINDOWS
#define BT_DEBUG_BREAK __debugbreak()
#else
#define BT_DEBUG_BREAK
#endif

#ifdef BT_DEBUG
#define BT_ENABLE_ASSERTS
#endif

#define BT_ENABLE_VERIFY

#ifdef BT_ENABLE_ASSERTS
#define BT_ASSERT(cond, ...) BT_CONTRACT_FAIL(::Bolt::ContractViolationKind::Assert, (cond), __VA_ARGS__)
#define BT_CORE_ASSERT(cond, ...) BT_CONTRACT_FAIL(::Bolt::ContractViolationKind::CoreAssert, (cond), __VA_ARGS__)
#else
#define BT_ASSERT(cond, ...) (cond)
#define BT_CORE_ASSERT(cond, ...) (cond)
#endif

#ifdef BT_ENABLE_VERIFY
#define BT_VERIFY(cond, ...) BT_CONTRACT_FAIL(::Bolt::ContractViolationKind::Verify, (cond), __VA_ARGS__)
#define BT_CORE_VERIFY(cond, ...) BT_CONTRACT_FAIL(::Bolt::ContractViolationKind::CoreVerify, (cond), __VA_ARGS__)
#else
#define BT_VERIFY(cond, ...) (cond)
#define BT_CORE_VERIFY(cond, ...) (cond)
#endif