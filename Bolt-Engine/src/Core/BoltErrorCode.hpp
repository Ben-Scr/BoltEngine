#pragma once

namespace Bolt {

	enum class BoltErrorCode {
		InvalidArgument,
		NotInitialized,
		AlreadyInitialized,
		FileNotFound,
		InvalidHandle,
		OutOfRange,
		OutOfBounds,
		Overflow,
		NullReference,
		LoadFailed,
		InvalidValue,
		Undefined
	};

} // namespace Bolt
