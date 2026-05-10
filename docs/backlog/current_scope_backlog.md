
Threading support primitives:

1. DONE: Define primitive wrapper boundary
2. DONE: Implement hardware thread count query
2a. DONE: Implement a hardware thread identification query
3. DONE: Implement mutex wrapper
5. DONE: Decide whether semaphore is needed now
7. DONE: Implement 2-phase parking gate
8. DONE: Define native thread entry contract
9. DONE: Implement native thread creation wrapper
10. DONE: Implement minimal thread start trampoline

DONE: Reorganise all the cross-platform support into the platform directories
DONE: Move the tga support into image/codec/
DONE: convert module binding to class

DONE: cleanup the platform defines
DONE: stop leakage of platform defines
DONE: use Linux path for Android wait word but add additional defines and warnings

DONE: CStaticLookup for provisioning and other data uintptr_t based

DONE: Wrapped phase gate (parking), wait predicates and counter semaphore

DONE: Native thread naming

DONE - DECIDED TO IGNORE EVERYTHING BUT PRIORITY AT THIS STAGE: add affinity, priority and numa identity

4. IN PROGRESS - (CONSIDERING ANDROID OPTIONS): Implement wait/wake wrapper or fallback-compatible wait primitive
6. IN PROGRESS - (IT IS NEEDED AS FALLBACK): Implement semaphore wrapper if needed


Remaining short term tasks:

Add the high performance counter

Add tga testing or perform some manual testing (leaning toward the latter)

Add the low level text ingester (using SuiteUTF)

Threading related, grouped here in order:
    1. Refactor the module and thread ids to separate role and sub-role/sub-identity
    2. Module, thread and system names (ID registry)
    3. Thread provisioning and other data access structures
    4. TLS definition (will depend on what the provisioning ends up looking like)

Build out the debug system (depends on completion of the threading related tasks)

Setup project to build on Linux

Create the Windows platform module

Create the Linux platform module

Longer term tasks:

Add smoke/stress tests around each layer of the threading (may become a redundant task if everything ends up being tested through actual use)


