# OACC: Order-Agnostic Constexpr Configuration with Compile-Time Uniqueness Checking

**A type-routed parameter pattern that accepts configuration arguments in any order and rejects duplicates at compile time**

**Date**: January 2026 (revised)
**Author**: Nihilai Collective Corp

---

## What this is, in one paragraph

OACC is a C++20/23 configuration pattern. Each setting is wrapped in its own type; a variadic generator accepts those wrappers in any order, routes each to the right field by overload resolution, and uses a concept to reject calls that pass the same setting type twice. Under `consteval` the whole thing evaluates during compilation and leaves no runtime trace. None of the ingredients are new: strong type wrappers, `consteval` member updates, fold expressions, and concept constraints are all standard C++20. The contribution is the combination: order-independent configuration with compile-time duplicate rejection, implemented as a library with no language extension. This document explains the pattern, states precisely what it guarantees and under what conditions, and backs each behavioral claim with compiler output.

---

## The problem

Configuration with many independent parameters (5–20+) wants several things at once: type safety, freedom to pass arguments in any order, early detection of mistakes like passing the same setting twice, and no runtime cost. The common approaches each give up one of these:

- **Positional arguments** (`create_config(8192, true, 4096, …)`): concise, but order-sensitive, unreadable at the call site, easy to transpose two arguments of the same type, and they cannot even express "the same parameter twice."
- **Builder pattern** (`.set_x(…).set_y(…)`): order-independent and readable, but a repeated setter is last-write-wins (a silent bug, detectable only at runtime if at all), and it needs a setter method per field.
- **Designated initializers** (`Config{ .a = …, .b = … }`, C++20): readable and checked, and a repeated field *is* a compile error, but the field order must match the struct declaration, and there's no place to hang cross-field validation or composition.
- **Named parameters (P0671)**: the best call-site ergonomics and the natural home for this, but not in the standard (targeting C++26 at the earliest), so unavailable to code that ships today.

These are trade-offs, not disqualifications. If you have two or three parameters always passed in the same order, positional arguments are simpler and you should use them. If runtime duplicate detection is acceptable, a builder is fine. OACC is worth its machinery specifically when you have many parameters, want order independence, and want a duplicate to be a build error rather than a silent overwrite, in C++20, without waiting for a language feature.

---

## The pattern

### Strongly-typed wrappers

Each setting is its own type, so the type system can route it. Scoped enums work well as value carriers (the same role `std::byte` plays in the standard library):

```cpp
enum class exceptions_type         : bool     { disabled = false, enabled = true };
enum class max_batch_size_type     : uint64_t {};
enum class max_context_length_type : uint64_t {};
enum class dev_type                : bool     { disabled = false, enabled = true };
```

### One update overload per setting

The config struct holds the fields and a `consteval update()` overload per wrapper type, each selected by `std::same_as` so overload resolution picks the right one and each touches only its own field:

```cpp
struct model_config {
    exceptions_type         exceptions{};
    max_batch_size_type     max_batch_size{ static_cast<max_batch_size_type>(1) };
    max_context_length_type max_context_length{ static_cast<max_context_length_type>(1024) };
    dev_type                dev{};

    template<std::same_as<max_batch_size_type> V>
    consteval auto update(V v) const { auto r{*this}; r.max_batch_size = v; return r; }
    // … one overload per wrapper type …
};
```

Because each `update` writes one field and reads none of the others, applying them in any order yields the same result: the operations commute. That is the basis of the order-independence claim, and it depends on keeping the updates genuinely independent (see "When order starts to matter," below).

### The generator, with the uniqueness check in a requires-clause

This is the load-bearing detail. The concept must constrain the **argument pack as a whole**, via a `requires` clause:

```cpp
template<typename search_type, typename... check_types>
constexpr uint64_t type_occurrence_count =
    (static_cast<uint64_t>(std::is_same_v<std::remove_cvref_t<search_type>,
                                          std::remove_cvref_t<check_types>>) + ...);

template<typename... arg_types>
concept unique_configuration_types =
    ((type_occurrence_count<arg_types, arg_types...> == 1) && ...);

template<typename... arg_types>
    requires unique_configuration_types<arg_types...>
consteval auto generate_model_config(arg_types... args) {
    model_config config{};
    ((config = config.update(args)), ...);
    return config;
}
```

The fold in `type_occurrence_count` counts how many times a type appears in the pack; the concept asserts every type appears exactly once; the `requires` clause makes a call that violates this non-viable. If you instead write the constraint as `template<unique_configuration_types... arg_types>`, you constrain each argument *individually*, which is always satisfied since a type trivially appears once among only itself, and duplicates are **not** caught. The whole-pack `requires` form is what makes the guarantee real; verify your own call site rejects a duplicate rather than assuming it does.

---

## Verified behavior

Everything in this section was checked on **GCC 13.3.0, x86-64, `-std=c++23`**. We did not run Clang or MSVC; the pattern uses only standard C++20 features, but if you ship on those, reproduce these three checks there.

### 1. Order independence

Two calls with the same settings in different orders produce the same configuration. This compiles, and the `static_assert` holds:

```cpp
static constexpr auto a = generate_model_config(dev_type::enabled, max_batch_size_type{23});
static constexpr auto b = generate_model_config(max_batch_size_type{23}, dev_type::enabled);
static_assert(static_cast<uint64_t>(a.max_batch_size) == static_cast<uint64_t>(b.max_batch_size)
              && a.dev == b.dev);   // passes
```

### 2. Duplicates are rejected at compile time

Passing the same wrapper type twice makes the call non-viable. Verbatim GCC 13.3 output:

```
error: no matching function for call to
       'generate_model_config(max_batch_size_type, dev_type, max_batch_size_type)'
note: candidate: 'template<class ... arg_types>
       requires unique_configuration_types<arg_types ...>
       consteval auto generate_model_config(arg_types ...)'
note: constraints not satisfied
note: required for the satisfaction of 'unique_configuration_types<arg_types...>'
      [with arg_types = {max_batch_size_type, dev_type, max_batch_size_type}]
```

The diagnostic names the failing function, the unsatisfied concept, and the exact argument pack, including the duplicated `max_batch_size_type`. It does not, on its own, print "appears 2 times" in prose; the offending type is identified by its presence twice in the listed pack. If you want the literal occurrence count surfaced in the message, route the check through a `static_assert` helper that embeds the count as a template value argument; the bare concept gives you the pack, which is usually enough to see the problem.

### 3. Under consteval, the configuration erases completely

When the generator and updates are `consteval` and the arguments are constants, the configuration is computed at compile time and nothing remains at runtime. For a program that builds a config and prints one field, the entire body of `main` in the `-O2` output is:

```asm
movl    $23, %edx          ; the computed field value, already a literal
movl    $2,  %edi
xorl    %eax, %eax
leaq    .LC0(%rip), %rsi
call    __printf_chk@PLT
xorl    %eax, %eax
ret
```

There is no trace of `update`, `generate_model_config`, or the `model_config` struct; the field value appears as the immediate `$23`. The configuration system has zero runtime footprint in this context, because it ran entirely in the compiler.

### Scope of the "zero overhead" claim

"Zero runtime overhead" applies precisely to the `consteval` / compile-time-constant case verified above. It is not a general statement about all uses of the pattern:

| Context | Where it evaluates | Runtime cost |
|---|---|---|
| `consteval` generator, constant args | Compile time only | None, fully erased (verified) |
| `constexpr`, used in a constant expression | Compile time | None |
| `constexpr`, called with runtime values | Runtime | Small; typically inlined at `-O2`, but not guaranteed |
| Plain runtime (no `constexpr`) | Runtime | Struct copies through the `update` chain |

The uniqueness check itself is always free at runtime: it is a concept, evaluated during overload resolution, and never emits code regardless of context. The erasure of the *configuration values* is what depends on `consteval` / constant evaluation. Outside that, the pattern is an opportunity for zero-cost abstraction, not a guarantee.

---

## How it compares

| Pattern | Order-independent | Duplicate detection | Runtime cost (constant case) | Call-site readability | Available |
|---|---|---|---|---|---|
| **OACC** | Yes | Compile-time (build error) | None under `consteval` (verified) | Moderate (typed wrappers) | C++20 |
| Positional args | No | Cannot express | None | Low | Always |
| Builder | Yes | Runtime, or silent last-write-wins | Builder object unless optimized | Medium | Always |
| Designated initializers | No (struct order) | Compile error, terse message | None | Good | C++20 |
| Named params (P0671) | Yes | Compile-time | None | Best | C++26+ (proposed) |

The honest positioning:

- **vs positional args**: OACC adds order independence, self-documenting call sites, and duplicate rejection, at the cost of defining a wrapper type per setting.
- **vs builder**: OACC turns a silent (or runtime) duplicate into a build error, and drops the builder object, at the cost of the wrapper types and one `update` overload each.
- **vs designated initializers**: aggregate init is simpler and also rejects duplicate fields, but it is order-constrained to the struct layout and has nowhere to attach validation or composition. OACC is order-free and extensible. If you don't need either, designated initializers are the lighter choice.
- **vs P0671**: named parameters would be more readable and are the proper language-level answer. They are not available yet. OACC delivers the safety part (order independence plus duplicate rejection) in C++20 today. When P0671 lands, the readability case for it is stronger.

We are not aware of another library-only pattern that provides order-independent parameters *and* compile-time duplicate rejection in this form, which is the sense in which the combination is the contribution. Each individual ingredient is standard and unoriginal; we make no novelty claim about any of them.

---

## Worked applications

The pattern is domain-agnostic: anything with several independent settings. In each case a repeated wrapper type is a build error.

**Database connection:** `connection_string`, `ssl_mode`, `max_pool_size`, `connection_timeout` as wrappers; `generate_db_config(...)` in any order. Passing `max_pool_size` twice fails to compile.

**Graphics pipeline state:** `blend_mode`, `depth_test`, `cull_mode`. Mutually exclusive choices belong *inside* one enum (`cull_mode{ none, front, back }`), not as two arguments of the same type, which the uniqueness check would reject anyway, correctly.

**Build configuration:** `optimization_level`, `debug_symbols`, `warnings_as_errors`, passed in any order.

---

## Limitations and trade-offs

- **The constraint must be a `requires` clause over the pack.** As shown above, constraining each argument individually silently disables duplicate detection. This is the one place the pattern is easy to get wrong; test that a duplicate actually fails.
- **One wrapper type and one `update` overload per setting.** This is the pattern's boilerplate. C++26 reflection could generate it; today it is written by hand.
- **Every parameter needs a distinct type.** Two raw `uint32_t` arguments are ambiguous and defeat the routing; wrap them (`enum class width : uint32_t {}` / `height`). This is a feature (it also prevents transposing same-typed arguments), but it is real work.
- **Order independence assumes independent updates.** If an `update` reads another field (e.g. clamping generation length to context length), the operations no longer commute and order matters again. Keep updates field-local, or sequence dependent steps explicitly and validate after.
- **"Zero overhead" is the `consteval`/constant case.** Verified there (see above). Runtime use with non-constant values is efficient-if-inlined, not guaranteed-free.
- **Verified on GCC 13.3 only.** Standard C++20 throughout; reproduce on your target toolchain if it matters.

---

## Summary

OACC is a practical assembly of standard C++20 features into a configuration interface with three properties: order-independent arguments, compile-time rejection of duplicate settings, and, under `consteval`, complete erasure of the configuration machinery, leaving the field values as literals in the output. The first two were verified by compiling valid and invalid calls and reading the diagnostics; the third by inspecting the emitted assembly. The single subtlety that makes or breaks the pattern is applying the uniqueness concept to the whole argument pack through a `requires` clause; applied per-argument it compiles but checks nothing. Where a simpler tool fits (few parameters, fixed order, or runtime checking being acceptable), this machinery is not warranted, and we have said where those lines fall.

---

*Copyright 2026 Nihilai Collective Corp. Licensed under the MIT License.*