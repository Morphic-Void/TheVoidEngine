
Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
License: MIT (see LICENSE file in repository root)

# TUnorderedSlots<TIndex>

## Overview

TUnorderedSlots<TIndex> maintains an unordered index over slot indices.

The template stores metadata only (list links, slot state, and counts)
and does not store or access payload.

This template is intended as a base class, not a concrete container.
The derived class:

- owns payload storage
- implements payload movement
- controls capacity approval
- exposes the public API

Single-threaded.

---

## Scope

- Metadata only; no payload ownership
- No payload construction or destruction
- Payload movement semantics are defined by the derived class

---

## State model

Each slot is in exactly one steady-state category:

- loose  - occupied, identity-bearing
- empty  - available free space

Internal states:

- unassigned - transitional only
- terminator - internal sentinel

Structures:

- Loose  -> circular doubly-linked list
- Empty  -> circular doubly-linked list

Stable-state invariant:

    loose_count() + empty_count() == capacity()

No overlap between categories.

---

## Slot index

A slot index is a signed integer in:

    [0, capacity())

It addresses metadata in the base and payload in the derived class.

Slot indices are stable except during pack().

### Sentinel conventions

- -1 is never a valid slot index
- -1 may be returned as a failure sentinel

Visit identifiers are a separate domain used only for on_visit():

- loose slots use identifier -1
- empty slots use identifier -2

---

## Ownership boundary

Base owns:

- slot metadata
- structural bookkeeping
- slot lifecycle and categorisation

Derived owns:

- payload storage and lifetime
- payload relocation
- capacity policy
- public API

The base never inspects payload.

---

## Observation model

### Traversal order

Traversal order is defined strictly by list order.

- Traversal order does not define rank
- No rank information is implied during traversal

### Rank

Rank is defined over loose slots only (occupied-domain).

    rank(slot_index) == number of loose slots with lower slot index

Properties:

- Valid rank domain: [0, loose_count())
- Empty slots have no rank and return -1
- Rank is independent of traversal order

---

## Pack model

pack() performs compaction of loose slots only.

After completion:

- Loose payload occupies slot indices [0, loose_count())
- Remaining slots are empty
- Loose and empty lists are rebuilt in ascending slot index order

Non-goals:

- Does not preserve empty-slot payload
- Does not define or modify rank semantics
- Does not provide full-domain remapping

Only loose slots are identity-bearing. Empty slots are free space only.

---

## Virtual callbacks

The derived class provides:

- on_visit(slot_index, identifier)
- on_move_payload(source_index, target_index)
- on_reserve_empty(minimum_capacity, recommended_capacity)

### on_visit

Called during visit operations.

Identifier values:

- -1 for loose slots
- -2 for empty slots

### on_move_payload

Moves payload during coordinated compaction (pack()).

TUnorderedSlots contract:

- source_index != target_index
- source_index and target_index are non-negative
- source_index references a loose slot
- target_index is in [0, loose_count())
- target_index may overwrite empty-slot payload

Empty-slot payload preservation is not required.

The derived class must implement its own overwrite-safe behaviour if preservation is required.
A preserving implementation may use swap-like movement, but the template guarantees compaction only.

Migration note:

- TOrderedSlots may use -1 to denote temporary storage
- TUnorderedSlots does not use -1 here
- Code written for TOrderedSlots is usually safe here
- Code written only for TUnorderedSlots may be incorrect in TOrderedSlots

### on_reserve_empty

Handshake for capacity growth.

Inputs:

- minimum_capacity (must be satisfied)
- recommended_capacity (growth heuristic)

The derived class returns the capacity to apply after ensuring payload storage is ready.

Returning a value less than minimum_capacity causes failure.

---

## Re-entry guard

Structural re-entry during virtual callbacks is prohibited.

Only a restricted set of protected accessors is safe inside callbacks.
All other protected functions are unsafe.

Enforcement:

- Debug builds hard fail
- Release builds soft fail (return false or -1, no mutation)

Integrity checks are valid only in stable state.

No thread safety is provided.

---

## Internal layering model

Three layers:

Protected interface:

- validates state
- establishes and releases guard state

safe_* wrappers:

- perform guarded virtual dispatch

private_* helpers:

- core mutation and traversal logic
- assume validated preconditions
- do not acquire locks directly
- invoke callbacks only via safe_* wrappers

This allows batched mutation under a single guard.

---

## Mutation model

Structural mutation:

- pack()

During mutation:

- structure may be temporarily inconsistent
- integrity checks are valid only after completion
- execution occurs under a single guard

---

## Capacity model

Definitions:

- capacity()
- minimum_safe_capacity() == high_index() + 1
- index_limit()
- capacity_limit() == index_limit() + 1

minimum_safe_capacity() may be transiently inconsistent during mutation.

Operations:

- safe_resize(new_capacity)
- reserve_empty(slot_count)
- reserve_and_acquire(...)
- shrink_to_fit()

Guarantees:

- safe_resize() fails if new_capacity < high_index() + 1
- reserve operations do not invalidate occupied slots
- shrink_to_fit() is equivalent to safe_resize(high_index() + 1)

No automatic shrinking or compaction is performed.

---

## Lifecycle

Destructive:

- initialise(capacity)
- shutdown()
- clear()

clear() resets metadata and rebuilds the empty list without deallocation.

Non-destructive:

Capacity:

- safe_resize(...)
- reserve_empty(...)
- reserve_and_acquire(...)
- shrink_to_fit()

Slot operations:

- acquire(...)
- erase(slot_index)

erase() returns a loose slot to the empty list. Payload handling is
the responsibility of the derived class.

---

## Visit operations

Visits operate over one or more categories.

Each visited slot calls:

    on_visit(slot_index, identifier)

Identifier values:

- loose: -1
- empty: -2

---

## Invariants (stable state)

- Each slot is in exactly one category
- Category counts sum to capacity()
- Loose and empty lists are circular and bidirectional
- high_index() is the highest occupied index or -1
- peak_usage() and peak_index() are monotonic maxima

---

## Complexity

- acquire / erase: O(1)
- list operations: O(1)
- pack(): O(n)
- check_integrity(): O(n)

---

## Alignment with TOrderedSlots

TOrderedSlots and TUnorderedSlots define rank differently.

TOrderedSlots:

- rank is full-domain
- occupied and empty slots participate in remapping
- pack()/sort_and_pack() perform total reordering

TUnorderedSlots:

- rank is occupied-domain only
- empty slots have no rank and no identity
- pack() performs compaction only

Migration constraints:

- Do not assume empty-slot payload preservation
- Do not assume empty-slot remapping
- Do not assume full-domain rank behaviour
- Do not assume shared callback contracts are identical

---

## Type constraints

Supported types:

- std::int32_t
- std::int16_t

TIndex must be signed.

Slot metadata representation is constrained for compact flag/index encoding.

Slot must be trivially copyable.

---

## Out of scope

The template does not:

- provide thread safety
- manage payload memory
- provide ordering or key comparison
- validate on_move_payload correctness
- auto-shrink without explicit calls