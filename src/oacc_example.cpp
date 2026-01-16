/*
 * Copyright 2026 Nihilai Collective Corp
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// oacc_example.cpp

#include "config.hpp"
#include <iostream>
#include <cstdint>
#include <limits>
#include <array>

// Undefined template - triggers compiler error with enum_error and values embedded in the type name
// This creates readable compile-time error messages that show exactly what went wrong
template<auto enum_error, auto... values> struct error_printer_impl_val;

// Static assertion wrapper that generates compile-time errors with contextual information
// Uses immediately-invoked constexpr lambda to trigger error_printer_impl_val when condition fails
template<bool value, auto enum_error, auto... values> struct static_assert_printer_val {
	static constexpr bool impl{ [] {
		if constexpr (!value) {
			error_printer_impl_val<enum_error, values...>::nonexistent_value;
			return false;
		} else {
			return true;
		}
	}() };
};

// Undefined template - triggers compiler error with enum_error and values embedded in the type name
// This creates readable compile-time error messages that show exactly what went wrong
template<auto enum_error, typename... values> struct error_printer_impl;

// Static assertion wrapper that generates compile-time errors with contextual information
// Uses immediately-invoked constexpr lambda to trigger error_printer_impl_val when condition fails
template<bool value, auto enum_error, typename... types> struct static_assert_printer {
	static constexpr bool impl{ [] {
		if constexpr (!value) {
			error_printer_impl<enum_error, types...>::nonexistent_value;
			return false;
		} else {
			return true;
		}
	}() };
};

// Strongly-typed configuration wrappers - each enum class becomes a unique type for overload resolution
// Using enum class as semantic wrappers enables type-based dispatch while preventing parameter confusion
// The disabled/enabled pattern provides compile-time optionality without runtime branches

enum class exceptions_type : bool {
	disabled = std::numeric_limits<bool>::min(),
	enabled	 = std::numeric_limits<bool>::max(),
};

enum class benchmark_type : bool {
	disabled = std::numeric_limits<bool>::min(),
	enabled	 = std::numeric_limits<bool>::max(),
};

enum class dev_type : bool {
	disabled = std::numeric_limits<bool>::min(),
	enabled	 = std::numeric_limits<bool>::max(),
};

// Value-carrying configuration types - enum class acts as strong typedef for type-based routing
// Using numeric_limits sentinels for disabled/enabled establishes "unset" vs "explicitly set" semantics

enum class max_context_length_type : uint64_t {
	disabled = std::numeric_limits<uint64_t>::min(),
	enabled	 = std::numeric_limits<uint64_t>::max(),
};

enum class gpu_rank_type : uint64_t {
	disabled = std::numeric_limits<uint64_t>::min(),
	enabled	 = std::numeric_limits<uint64_t>::max(),
};

enum class gpu_count_type : uint64_t {
	disabled = std::numeric_limits<uint64_t>::min(),
	enabled	 = std::numeric_limits<uint64_t>::max(),
};

enum class max_generation_length_type : uint64_t {
	disabled = std::numeric_limits<uint64_t>::min(),
	enabled	 = std::numeric_limits<uint64_t>::max(),
};

enum class max_prompt_length_type : uint64_t {
	disabled = std::numeric_limits<uint64_t>::min(),
	enabled	 = std::numeric_limits<uint64_t>::max(),
};

enum class max_batch_size_type : uint64_t {
	disabled = std::numeric_limits<uint64_t>::min(),
	enabled	 = std::numeric_limits<uint64_t>::max(),
};

// Configuration container with default values
// Each field corresponds to exactly one wrapper type above
struct model_config {
	exceptions_type exceptions{};
	max_context_length_type max_context_length{ static_cast<max_context_length_type>(1024) };
	max_prompt_length_type max_prompt_length{ static_cast<max_prompt_length_type>(std::numeric_limits<uint64_t>::max()) };
	max_generation_length_type max_generation_length{ static_cast<max_generation_length_type>(std::numeric_limits<uint64_t>::max()) };
	max_batch_size_type max_batch_size{ static_cast<max_batch_size_type>(1) };
	gpu_count_type gpu_count{ static_cast<gpu_count_type>(1ull) };
	gpu_rank_type gpu_rank{};
	benchmark_type benchmark{};
	dev_type dev{};

	// Type-specific update methods - each overload handles exactly one wrapper type
	// Overload resolution routes each parameter to the correct update function at compile time
	// consteval forces compile-time evaluation, ensuring zero runtime overhead
	// Each update modifies only its corresponding field (disjoint state) enabling order independence

	template<std::same_as<exceptions_type> value_type> consteval auto update(const value_type value) const {
		model_config return_value{ *this };
		return_value.exceptions = value;
		return return_value;
	}

	template<std::same_as<max_context_length_type> value_type> consteval auto update(const value_type value) const {
		model_config return_value{ *this };
		return_value.max_context_length = value;
		return return_value;
	}

	template<std::same_as<gpu_rank_type> value_type> consteval auto update(const value_type value) const {
		model_config return_value{ *this };
		return_value.gpu_rank = value;
		return return_value;
	}

	template<std::same_as<gpu_count_type> value_type> consteval auto update(const value_type value) const {
		model_config return_value{ *this };
		return_value.gpu_count = value;
		return return_value;
	}

	template<std::same_as<max_prompt_length_type> value_type> consteval auto update(const value_type value) const {
		model_config return_value{ *this };
		return_value.max_prompt_length = value;
		return return_value;
	}

	template<std::same_as<max_generation_length_type> value_type> consteval auto update(const value_type value) const {
		model_config return_value{ *this };
		return_value.max_generation_length = value;
		return return_value;
	}

	template<std::same_as<max_batch_size_type> value_type> consteval auto update(const value_type value) const {
		model_config return_value{ *this };
		return_value.max_batch_size = value;
		return return_value;
	}

	template<std::same_as<benchmark_type> value_type> consteval auto update(const value_type value) const {
		model_config return_value{ *this };
		return_value.benchmark = value;
		return return_value;
	}

	template<std::same_as<dev_type> value_type> consteval auto update(const value_type value) const {
		model_config return_value{ *this };
		return_value.dev = value;
		return return_value;
	}
};

constexpr uint64_t ceil_div(uint64_t a, uint64_t b) noexcept {
	return (a + b - 1) / b;
}

// Compile-time value transformation - if a parameter is set to max (sentinel for "unset"),
// compute a reasonable default based on another parameter
// This demonstrates dependent defaults while maintaining compile-time evaluation
template<uint64_t value_01, uint64_t value_02> consteval uint64_t get_updated_value() {
	if constexpr (value_01 == std::numeric_limits<uint64_t>::max()) {
		return ceil_div(value_02, 2);
	} else {
		return value_01;
	}
}

// Error categories for static_assert messages
enum class model_config_errors {
	context_length_too_large,
	context_length_too_short,
	prompt_length_or_generation_length_too_large,
	duplicate_type_input,
};

// Compile-time configuration validator and type generator
// Takes a compile-time model_config and produces constexpr constants with validation
// All static_asserts fire at compile time if constraints are violated
template<const model_config& config> struct model_config_type {
	static constexpr bool exceptions				= static_cast<bool>(config.exceptions);
	static constexpr uint64_t max_context_length	= static_cast<uint64_t>(config.max_context_length);
	static constexpr uint64_t max_prompt_length		= get_updated_value<static_cast<uint64_t>(config.max_prompt_length), max_context_length>();
	static constexpr uint64_t max_generation_length = get_updated_value<static_cast<uint64_t>(config.max_generation_length), max_context_length>();
	static constexpr uint64_t max_batch_size		= static_cast<uint64_t>(config.max_batch_size);
	static constexpr uint64_t gpu_count				= static_cast<uint64_t>(config.gpu_count);
	static constexpr uint64_t gpu_rank				= static_cast<uint64_t>(config.gpu_rank);
	static constexpr bool benchmark					= static_cast<bool>(config.benchmark);
	static constexpr bool dev						= static_cast<bool>(config.dev);

	// Compile-time validation - these static_asserts fire during template instantiation
	// If constraints fail, compilation aborts with clear error messages showing exact values
	static_assert(static_assert_printer_val<(max_context_length > 1), model_config_errors::context_length_too_short, max_context_length>::impl);
	static_assert(static_assert_printer_val<(max_generation_length + max_prompt_length) <= max_context_length, model_config_errors::prompt_length_or_generation_length_too_large,
		max_context_length, max_generation_length, max_prompt_length>::impl);

	static constexpr const model_config& get_config() {
		return config;
	}
};

// CRITICAL INNOVATION: Compile-time uniqueness checking via fold-expression-based occurrence counting
// For a given search_type, count how many times it appears in check_types parameter pack
// Fold expression expands to: is_same<T, T1> + is_same<T, T2> + ... + is_same<T, Tn>
// Result: integer count of how many times search_type appears in the pack
template<typename search_type, typename... check_types> constexpr uint64_t type_occurrence_count =
	(static_cast<uint64_t>(std::is_same_v<std::remove_cvref_t<search_type>, std::remove_cvref_t<check_types>>) + ...);

// Concept that enforces each type in arg_types appears exactly once
// Expands to: (count<T1, all> == 1) && (count<T2, all> == 1) && ... && (count<Tn, all> == 1)
// If any type appears more than once, concept fails and compilation aborts with clear diagnostic
// This is the KEY ADVANCEMENT: compile-time duplicate parameter detection
template<typename... arg_types>
concept unique_configuration_types = ((type_occurrence_count<arg_types, arg_types...> == 1) && ...);

// Variadic configuration generator with uniqueness constraint
// Parameters can be provided in ANY order - overload resolution routes each to correct update()
// The unique_configuration_types concept ensures no parameter type appears twice
// Fold expression applies updates sequentially: config.update(arg1).update(arg2).update(arg3)...
// consteval forces compile-time evaluation - entire configuration system has zero runtime cost
template<unique_configuration_types... arg_types> inline static consteval auto generate_model_config(arg_types... args) {
	static_assert(static_assert_printer<unique_configuration_types<arg_types...>, model_config_errors::duplicate_type_input, arg_types...>::impl);
	model_config config_new{};
	((config_new = config_new.update(args)), ...);
	return config_new;
};

// Overload that takes existing config as base and applies additional updates
// Enables configuration composition and hierarchical defaults
// Uniqueness checking applies only to new parameters, not base config fields
template<unique_configuration_types... arg_types> inline static consteval auto generate_model_config(model_config config_new, arg_types... args) {
	static_assert(static_assert_printer<unique_configuration_types<arg_types...>, model_config_errors::duplicate_type_input, arg_types...>::impl);
	((config_new = config_new.update(args)), ...);
	return config_new;
};

// Demonstration: parameters can be provided in any order, or omitted entirely
// If the same parameter type appears twice, compilation fails with clear error message
int main() {
	// Default configuration - all parameters use struct defaults
	static constexpr auto config_01 = generate_model_config();

	// Parameters in arbitrary order - type-based dispatch routes each correctly
	// Try adding a duplicate parameter (e.g., dev_type::enabled at the end) to see compile error
	static constexpr auto config_02 = generate_model_config(dev_type::disabled, benchmark_type::disabled, max_batch_size_type{ 23 });

	// Parameters in arbitrary order - type-based dispatch routes each correctly
	// Try adding a duplicate parameter (e.g., dev_type::enabled at the end) to see compile error
	static constexpr auto config_03 = generate_model_config(benchmark_type::disabled, max_context_length_type{ 23 }, dev_type::enabled);

	// Configuration fully evaluated at compile time - zero runtime overhead
	using model_config_type = model_config_type<config_01>;
	static constexpr model_config_type model_config_val{};

	return 0;
}