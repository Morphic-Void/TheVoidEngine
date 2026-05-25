
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   host.hpp
//  Author: Ritchie Brannan
//  Date:   15 May 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  The main host service thread for the engine.
//
//  This is currently only a test/sketch/prototype
//  to validate the existing codebase features.
//
//  The code is placeholder and not final.

#include <atomic>       //  std::atomic
#include <cstdint>      //  std::int32_t, std::uint32_t
#include <thread>       //  std::this_thread::yield
#include <utility>      //  std::move

#include "host/host.hpp"
#include "containers/containers.hpp"
#include "image/codec/tga.hpp"
#include "platform/filesystem/file.hpp"
#include "platform/module/binding.hpp"
#include "platform/path/native_path.hpp"
#include "platform/system/performance_counter.hpp"
#include "platform/system/process_priority.hpp"
#include "platform/threading/platform_threading.hpp"
#include "system/system_ids.hpp"
#include "system/TStaticLookup.hpp"
#include "threading/threading.hpp"
#include "types/typeless_traits.hpp"
#include "types/typeless_pod.hpp"
#include "types/typeless.hpp"

#include "debug/debug.hpp"

namespace type_ids
{   //  will be moved into system/system_ids.hpp

static constexpr std::size_t msg_id_unrecognised_msg = encode_id(100u);

static constexpr std::size_t msg_id_file_load_request = encode_id(101u);
static constexpr std::size_t msg_id_file_save_request = encode_id(102u);
static constexpr std::size_t msg_id_tga_load_request = encode_id(103u);
static constexpr std::size_t msg_id_tga_save_request = encode_id(104u);
static constexpr std::size_t msg_id_tga_encode_request = encode_id(105u);
static constexpr std::size_t msg_id_tga_decode_request = encode_id(106u);

static constexpr std::size_t msg_id_file_load_result = encode_id(107u);
static constexpr std::size_t msg_id_file_save_result = encode_id(108u);
static constexpr std::size_t msg_id_tga_load_result = encode_id(109u);
static constexpr std::size_t msg_id_tga_save_result = encode_id(110u);

static constexpr std::size_t msg_id_file_load_result_owning = encode_id(111u);
static constexpr std::size_t msg_id_tga_encode_result_owning = encode_id(112u);
static constexpr std::size_t msg_id_tga_decode_result_owning = encode_id(113u);

};

struct UnrecognisedMsg { std::size_t msg_id; };

struct FileLoadRequest { const char* file; };
struct FileSaveRequest { const char* file; CByteConstView* view; };
struct TgaLoadRequest { const char* file; bool vflip; };
struct TgaSaveRequest { const char* file; image::codec::tga::EncodeOptions* options; CByteRectConstView* view; };
struct TgaEncodeRequest { CByteRectConstView* view; image::codec::tga::EncodeOptions* options; };
struct TgaDecodeRequest { CByteConstView* view; bool vflip; };

struct FileLoadResult { CByteConstView* view; };
struct FileSaveResult { bool success; };
struct TgaLoadResult { CByteRectConstView* view; image::codec::tga::decoded_image_desc desc; };
struct TgaSaveResult { bool success; };

struct FileLoadResultOwning { std::size_t async_slot; CByteBuffer buffer; };
struct TgaEncodeResultOwning { std::size_t async_slot; CByteBuffer buffer; };
struct TgaDecodeResultOwning { std::size_t async_slot; CByteRectBuffer buffer; image::codec::tga::decoded_image_desc desc; };

MV_DECLARE_TYPELESS(UnrecognisedMsg, type_ids::msg_id_unrecognised_msg);

MV_DECLARE_TYPELESS(FileLoadRequest, type_ids::msg_id_file_load_request);
MV_DECLARE_TYPELESS(FileSaveRequest, type_ids::msg_id_file_save_request);
MV_DECLARE_TYPELESS(TgaSaveRequest, type_ids::msg_id_tga_save_request);
MV_DECLARE_TYPELESS(TgaLoadRequest, type_ids::msg_id_tga_load_request);
MV_DECLARE_TYPELESS(TgaEncodeRequest, type_ids::msg_id_tga_encode_request);
MV_DECLARE_TYPELESS(TgaDecodeRequest, type_ids::msg_id_tga_decode_request);

MV_DECLARE_TYPELESS(FileLoadResult, type_ids::msg_id_file_load_result);
MV_DECLARE_TYPELESS(FileSaveResult, type_ids::msg_id_file_save_result);
MV_DECLARE_TYPELESS(TgaLoadResult, type_ids::msg_id_tga_load_result);
MV_DECLARE_TYPELESS(TgaSaveResult, type_ids::msg_id_tga_save_result);

MV_DECLARE_TYPELESS(FileLoadResultOwning, type_ids::msg_id_file_load_result_owning);
MV_DECLARE_TYPELESS(TgaEncodeResultOwning, type_ids::msg_id_tga_encode_result_owning);
MV_DECLARE_TYPELESS(TgaDecodeResultOwning, type_ids::msg_id_tga_decode_result_owning);

struct ThreadConfig
{
    std::size_t system_id{ 0u };
    const char* name{ nullptr };
    platform::threading::EThreadPriority priority{ platform::threading::EThreadPriority::Normal };
    platform::threading::FThreadEntry entry_point{ nullptr };
};

class CThreadResources
{
public:
    CThreadResources(const ThreadConfig& thread_config) noexcept : config(thread_config) {}
    ~CThreadResources() noexcept = default;
    bool created{ false };
    platform::threading::CThread thread;
    threading::CParkingTicket parking_ticket;
    threading::CWaitPredicate wait_predicate;           //  use wait_predicate for simple workers
    threading::CCountingSemaphore counting_semaphore;   //  use counting_semaphore for multi-thread jobs
    threading::transports::TQueue<threading::CPodThreadMsg> host_to_worker_msgs;
    threading::transports::TQueue<threading::CPodThreadMsg> worker_to_host_msgs;
    threading::transports::TOwning<memory::CTypeless> worker_owned_to_host_owned;
    threading::CThreadControlState control_state;
    ThreadConfig config;
};

class CThreadContext
{
public:
    CThreadContext(CThreadResources& resources) noexcept;
    ~CThreadContext() noexcept = default;

    const char* get_name() const noexcept { return m_resources.config.name; }

    void startup() noexcept;
    void mark_waiting() noexcept { m_resources.control_state.mark_waiting(); }
    void mark_running() noexcept { m_resources.control_state.mark_running(); }
    void mark_exiting() noexcept { m_resources.control_state.mark_exiting(); }
    void mark_exited() noexcept { m_resources.control_state.mark_exited(); }
    void mark_failed(const std::uint32_t code) noexcept { return m_resources.control_state.mark_failed(code); }
    void advance_heartbeat() noexcept { m_resources.control_state.advance_heartbeat(); }
    bool exit_requested() const noexcept { return m_resources.control_state.exit_requested(); }
    std::uint32_t wait_for_new_epoch(const uint32_t epoch) noexcept;
    bool read(threading::CPodThreadMsg& msg) noexcept { return m_resources.host_to_worker_msgs.read(msg); }
    bool post(const threading::CPodThreadMsg& msg) noexcept { return m_resources.worker_to_host_msgs.post(msg); }
    bool pass_ownership(memory::CTypeless& obj) noexcept { return m_resources.worker_owned_to_host_owned.post(std::move(obj)); }
private:
    CThreadResources& m_resources;
};

inline CThreadContext::CThreadContext(CThreadResources& resources) noexcept :
    m_resources{ resources }
{
}

inline void CThreadContext::startup() noexcept
{
    m_resources.control_state.mark_startup();
    std::size_t system_id = m_resources.config.system_id;   //  This needs to be stored in TLS for the real implementation
    (void)platform::threading::set_current_thread_name(m_resources.config.name);
    (void)platform::threading::set_current_thread_priority(m_resources.config.priority);
}

std::uint32_t CThreadContext::wait_for_new_epoch(const uint32_t epoch) noexcept
{
    debug_utils::debug_output("Worker (%s): Waiting epoch=%u\n", m_resources.config.name, epoch);

    mark_waiting();
    std::uint32_t new_epoch = m_resources.wait_predicate.wait_until_not_equal(m_resources.parking_ticket, epoch);
    mark_running();

    debug_utils::debug_output("Worker (%s): Running epoch=%u\n", m_resources.config.name, new_epoch);

    return new_epoch;
}

class CThreadPackage
{
public:
    CThreadPackage(const ThreadConfig& thread_config) noexcept : m_resources(thread_config), m_context(m_resources) {}
    ~CThreadPackage() noexcept = default;
    bool startup() noexcept;
    bool shutdown() noexcept;
    bool take_ownership(memory::CTypeless& obj) noexcept;
    bool read(threading::CPodThreadMsg& msg) noexcept;
    bool post(const threading::CPodThreadMsg& msg) noexcept;
    threading::EThreadRunState query_state() const noexcept;
    CThreadContext& get_context() noexcept { return m_context; }
private:
    CThreadResources m_resources;
    CThreadContext m_context;
};

inline bool CThreadPackage::startup() noexcept
{
    m_resources.control_state.mark_pending_start();
    if (m_resources.wait_predicate.acquire_control())
    {
        if (m_resources.host_to_worker_msgs.initialise_growable(0u))
        {
            if (m_resources.worker_to_host_msgs.initialise_growable(0u))
            {
                if (m_resources.worker_owned_to_host_owned.initialise(0u))
                {
                    m_resources.created = m_resources.thread.create(m_resources.config.entry_point, &get_context());
                    if (m_resources.created)
                    {
                        threading::EThreadRunState state;
                        for (;;)
                        {
                            state = query_state();
                            if ((state == threading::EThreadRunState::Waiting) ||
                                (state == threading::EThreadRunState::Running) ||
                                (state == threading::EThreadRunState::Exited) ||
                                (state == threading::EThreadRunState::Failed))
                            {
                                break;
                            }
                            std::this_thread::yield();
                        }
                        if (m_resources.control_state.query_ready())
                        {
                            return true;
                        }
                        m_resources.created = false;
                    }
                    m_resources.worker_owned_to_host_owned.deallocate();
                }
                m_resources.worker_to_host_msgs.deallocate();
            }
            m_resources.host_to_worker_msgs.deallocate();
        }
        m_resources.wait_predicate.release_control();
    }
    return false;
}

inline bool CThreadPackage::shutdown() noexcept
{
    if (m_resources.created)
    {
        m_resources.control_state.request_exit();
        m_resources.wait_predicate.release_control();
        for (;;)
        {   //  wait for the worker to either exit or re-enter a waiting state
            threading::EThreadRunState state = query_state();
            if ((state == threading::EThreadRunState::Exited) || (state == threading::EThreadRunState::Failed))
            {
                break;
            }
            std::this_thread::yield();
        }
        m_resources.created = m_resources.thread.join_and_close();
        if (!m_resources.created)
        {
            m_resources.worker_owned_to_host_owned.deallocate();
            m_resources.worker_to_host_msgs.deallocate();
            m_resources.host_to_worker_msgs.deallocate();
        }
    }
    return m_resources.created;
}

inline bool CThreadPackage::take_ownership(memory::CTypeless& msg) noexcept
{
    return m_resources.worker_owned_to_host_owned.read(msg);
}

inline bool CThreadPackage::read(threading::CPodThreadMsg& msg) noexcept
{
    return m_resources.worker_to_host_msgs.read(msg);
}

inline bool CThreadPackage::post(const threading::CPodThreadMsg& msg) noexcept
{
    bool success = m_resources.host_to_worker_msgs.post(msg);
    if (success)
    {
        m_resources.wait_predicate.poke_epoch_and_wake_one();
    }
    return success;
}

inline threading::EThreadRunState CThreadPackage::query_state() const noexcept
{
    return m_resources.control_state.query_state();
}

static std::uint32_t worker_thread(void* user_data) noexcept
{
    //  Initialise the context
    CThreadContext context = *reinterpret_cast<CThreadContext*>(user_data);

    debug_utils::debug_output("Worker (%s): Starting\n", context.get_name());

    //  Standard thread startup
    context.startup();

    std::uint32_t epoch = 0u;
    while (!context.exit_requested())
    {
        context.advance_heartbeat();
        threading::CPodThreadMsg inbound_msg;
        if (context.read(inbound_msg))
        {
            debug_utils::debug_output("Worker (%s): Message received\n", context.get_name());

            switch (inbound_msg.payload.query_type_id())
            {
                case (k_type_id_v<FileLoadRequest>):
                {
                    debug_utils::debug_output("Worker (%s): File load request\n", context.get_name());

                    FileLoadRequest request;
                    (void)inbound_msg.payload.copy_to(request);
                    memory::CTypeless outbound_msg = create_typeless<FileLoadResultOwning>();
                    FileLoadResultOwning& result = *typeless_cast<FileLoadResultOwning>(outbound_msg);
                    result.async_slot = inbound_msg.async_slot;
                    result.buffer = platform::filesystem::loadFile(request.file);
                    (void)context.pass_ownership(outbound_msg);
                    break;
                }
                case (k_type_id_v<FileSaveRequest>):
                {
                    debug_utils::debug_output("Worker (%s): File save request\n", context.get_name());

                    FileSaveRequest request;
                    (void)inbound_msg.payload.copy_to(request);
                    FileSaveResult result;
                    result.success = platform::filesystem::saveFile(request.file, *request.view);
                    threading::CPodThreadMsg outbound_msg;
                    outbound_msg.async_slot = inbound_msg.async_slot;
                    outbound_msg.payload.assign(result);
                    (void)context.post(outbound_msg);
                    break;
                }
                case (k_type_id_v<TgaEncodeRequest>):
                {
                    debug_utils::debug_output("Worker (%s): TGA encode request\n", context.get_name());

                    TgaEncodeRequest request;
                    (void)inbound_msg.payload.copy_to(request);
                    memory::CTypeless outbound_msg = create_typeless<TgaEncodeResultOwning>();
                    TgaEncodeResultOwning& result = *typeless_cast<TgaEncodeResultOwning>(outbound_msg);
                    result.async_slot = inbound_msg.async_slot;
                    result.buffer = image::codec::tga::encode(*request.view, *request.options);
                    (void)context.pass_ownership(outbound_msg);
                    break;
                }
                case (k_type_id_v<TgaDecodeRequest>):
                {
                    debug_utils::debug_output("Worker (%s): TGA decode request\n", context.get_name());

                    TgaDecodeRequest request;
                    (void)inbound_msg.payload.copy_to(request);
                    memory::CTypeless outbound_msg = create_typeless<TgaDecodeResultOwning>();
                    TgaDecodeResultOwning& result = *typeless_cast<TgaDecodeResultOwning>(outbound_msg);
                    result.async_slot = inbound_msg.async_slot;
                    result.buffer = image::codec::tga::decode(*request.view, result.desc, request.vflip);
                    (void)context.pass_ownership(outbound_msg);
                    break;
                }
                default:
                {
                    debug_utils::debug_output("Worker (%s): Unrecognised message type (%d)\n", context.get_name(), inbound_msg.payload.query_type_id());

                    UnrecognisedMsg unrecognised;
                    unrecognised.msg_id = inbound_msg.payload.query_type_id();
                    threading::CPodThreadMsg outbound_msg;
                    outbound_msg.async_slot = inbound_msg.async_slot;
                    outbound_msg.payload.assign(unrecognised);
                    (void)context.post(outbound_msg);
                    break;
                }
            }
        }
        else
        {
            epoch = context.wait_for_new_epoch(epoch);
        }
    }
    context.mark_exited();

    debug_utils::debug_output("Worker (%s): Exited\n", context.get_name());

    return 0u;
}

static std::uint32_t app_thread(void* user_data) noexcept
{
    //  Initialise the context
    CThreadContext context = *reinterpret_cast<CThreadContext*>(user_data);

    debug_utils::debug_output("Application: Starting\n");

    //  Standard thread startup
    context.startup();

    platform::system::CPerfCounter perf_counter;
    platform::system::CPerfCountConversion perf_count_converter;
    perf_counter.update();
    perf_count_converter.init();
    std::uint64_t ticks_per_second = perf_count_converter.query_ticks_per_second();

    struct AsyncTgaLoad
    {
        bool complete = false;
        bool success = false;
        CByteRectConstView view;
        image::codec::tga::decoded_image_desc desc = image::codec::tga::decoded_image_desc::RGBA;
    };

    struct AsyncTgaSave
    {
        bool complete = false;
        bool success = false;
    };

    AsyncTgaLoad tga_load;
    AsyncTgaSave tga_save;

    {   //  kick off the test
        TgaLoadRequest tga_load_request;
        tga_load_request.file = "d:/test_input.tga";
        tga_load_request.vflip = false;
        threading::CPodThreadMsg outbound_msg;
        outbound_msg.async_slot = 0;
        outbound_msg.payload.assign(tga_load_request);
        (void)context.post(outbound_msg);
    }

    enum class ETgaTestStates : std::uint32_t { no_state = 0u, waiting_for_tga_load, waiting_for_tga_save, done };
    ETgaTestStates tga_state = ETgaTestStates::waiting_for_tga_load;

    image::codec::tga::EncodeOptions tga_encode_options;

    context.mark_running();

    debug_utils::debug_output("Application: Running\n");

    while (!context.exit_requested())
    {
        std::uint64_t tick_delta = perf_counter.query_delta();
        if (tick_delta >= ticks_per_second)
        {
            context.advance_heartbeat();
            perf_counter.update();

            debug_utils::debug_output("Application: Heartbeat\n");
        }

        bool state_updated = false;

        while (!context.exit_requested())
        {   //  drain incoming messages

            threading::CPodThreadMsg inbound_msg;
            if (!context.read(inbound_msg))
            {
                break;
            }

            debug_utils::debug_output("Application: Message received\n");

            switch (inbound_msg.payload.query_type_id())
            {
                case (k_type_id_v<TgaLoadResult>):
                {
                    debug_utils::debug_output("Application: TGA load result\n");

                    TgaLoadResult tga_load_result;
                    (void)inbound_msg.payload.copy_to(tga_load_result);
                    tga_load.view = *tga_load_result.view;
                    tga_load.desc = tga_load_result.desc;
                    tga_load.success = !tga_load_result.view->is_empty();
                    tga_load.complete = true;
                    state_updated = true;
                    break;
                }
                case (k_type_id_v<TgaSaveResult>):
                {
                    debug_utils::debug_output("Application: TGA save result\n");

                    TgaSaveResult tga_save_result;
                    (void)inbound_msg.payload.copy_to(tga_save_result);
                    tga_save.success = tga_save_result.success;
                    tga_save.complete = true;
                    state_updated = true;
                    break;
                }
                default:
                {
                    debug_utils::debug_output("Application: Unrecognised message type (%d)\n", inbound_msg.payload.query_type_id());

                    UnrecognisedMsg unrecognised;
                    unrecognised.msg_id = inbound_msg.payload.query_type_id();
                    threading::CPodThreadMsg outbound_msg;
                    outbound_msg.async_slot = inbound_msg.async_slot;
                    outbound_msg.payload.assign(unrecognised);
                    (void)context.post(outbound_msg);
                    break;
                }
            }
        }

        if (state_updated)
        {
            if (tga_state == ETgaTestStates::waiting_for_tga_load)
            {
                if (tga_load.complete)
                {
                    if (!tga_load.success)
                    {
                        context.mark_failed(1u);
                        break;
                    }

                    tga_encode_options.src = (tga_load.desc == image::codec::tga::decoded_image_desc::Gray) ?
                        image::codec::tga::image_encode_src::Gray :
                        image::codec::tga::image_encode_src::AutoTrue32;

                    TgaSaveRequest tga_save_request;
                    tga_save_request.file = "d:/test_output.tga";
                    tga_save_request.view = &tga_load.view;
                    tga_save_request.options = &tga_encode_options;

                    threading::CPodThreadMsg outbound_msg;
                    outbound_msg.async_slot = 0;
                    outbound_msg.payload.assign(tga_save_request);
                    (void)context.post(outbound_msg);

                    tga_state = ETgaTestStates::waiting_for_tga_save;
                }
            }
            else if (tga_state == ETgaTestStates::waiting_for_tga_save)
            {
                if (tga_save.complete)
                {
                    if (!tga_save.success)
                    {
                        context.mark_failed(2u);
                        break;
                    }

                    tga_state = ETgaTestStates::done;
                    break;
                }
                break;  //  test force exit
            }
        }
    }
    context.mark_exited();

    debug_utils::debug_output("Application: Exited\n");

    return 0u;
}

int host()
{
    debug_utils::debug_output("Host: Starting\n");

    ThreadConfig thread_configs[3]{
        {system_ids::bg_file_io, "bg_file_io", platform::threading::EThreadPriority::Background, &worker_thread},
        {system_ids::bg_conditioning, "bg_conditioning", platform::threading::EThreadPriority::Background, &worker_thread},
        {system_ids::application, "application", platform::threading::EThreadPriority::Normal, &app_thread} };

    enum class EWorkerThreadID : std::uint8_t { bg_file_io = 0u, bg_conditioning, application };

    int32_t thread_slots[3]{};

    TUnorderedCollection<CThreadPackage> thread_packages;

    bool initialised = true;
    if (initialised) initialised = thread_packages.initialise();
    if (initialised)
    {
        for (std::int32_t thread_index = 0; thread_index <= 2; ++thread_index)
        {
            int32_t thread_slot = thread_packages.emplace(thread_configs[thread_index]);
            if (thread_slot < 0)
            {
                initialised = false;
                break;
            }
            thread_slots[thread_index] = thread_slot;
            CThreadPackage& package = *thread_packages.get_object(thread_slot);
            initialised = package.startup();
            if (!initialised)
            {
                break;
            }
        }
    }

    struct AsyncTgaSave
    {
        const char* file = nullptr;
        CByteBuffer buffer;
        CByteConstView view;
        CByteRectBuffer rect_buffer;
        CByteRectConstView rect_view;
        image::codec::tga::EncodeOptions options{};
    };

    struct AsyncTgaLoad
    {
        const char* file = nullptr;
        CByteBuffer buffer;
        CByteConstView view;
        CByteRectBuffer rect_buffer;
        CByteRectConstView rect_view;
        bool vflip = false;
        image::codec::tga::decoded_image_desc desc = image::codec::tga::decoded_image_desc::RGBA;
    };

    AsyncTgaLoad async_tga_load;
    AsyncTgaSave async_tga_save;

    if (initialised)
    {
        platform::system::CPerfCounter perf_counter;
        platform::system::CPerfCountConversion perf_count_converter;
        perf_counter.update();
        perf_count_converter.init();
        std::uint64_t ticks_per_second = perf_count_converter.query_ticks_per_second();

        CThreadPackage& application_package = *thread_packages.get_object(thread_slots[static_cast<std::uint8_t>(EWorkerThreadID::application)]);
        while (application_package.query_state() != threading::EThreadRunState::Exited)
        {
            std::uint64_t tick_delta = perf_counter.query_delta();
            if ((tick_delta * 500u) >= ticks_per_second)
            {
                perf_counter.update();

                debug_utils::debug_output("Host: Service OS Pump\n");
            }

            for (int32_t inbound_slot = thread_packages.first_live(); inbound_slot >= 0; inbound_slot = thread_packages.next_live(inbound_slot))
            {
                CThreadPackage& inbound_package = *thread_packages.get_object(inbound_slot);
                threading::CPodThreadMsg inbound_msg;
                while (inbound_package.read(inbound_msg))
                {
                    debug_utils::debug_output("Host: Recieved a message\n");

                    switch (inbound_msg.payload.query_type_id())
                    {
                        case (k_type_id_v<FileSaveResult>):
                        {
                            debug_utils::debug_output("Host: Recieved a file save result\n");

                            FileSaveResult result;
                            (void)inbound_msg.payload.copy_to(result);
                            const std::int32_t outbound_slot = thread_slots[static_cast<std::uint8_t>(EWorkerThreadID::application)];
                            CThreadPackage& outbound_package = *thread_packages.get_object(outbound_slot);
                            TgaSaveResult forward;
                            forward.success = result.success;
                            threading::CPodThreadMsg outbound_msg;
                            outbound_msg.async_slot = 0;
                            outbound_msg.payload.assign(forward);
                            (void)outbound_package.post(outbound_msg);
                            break;
                        }
                        case (k_type_id_v<TgaLoadRequest>):
                        {
                            debug_utils::debug_output("Host: Recieved a TGA load request\n");

                            TgaLoadRequest tga_load_request;
                            (void)inbound_msg.payload.copy_to(tga_load_request);
                            async_tga_load.file = tga_load_request.file;
                            async_tga_load.vflip = tga_load_request.vflip;
                            const std::int32_t outbound_slot = thread_slots[static_cast<std::uint8_t>(EWorkerThreadID::bg_file_io)];
                            CThreadPackage& outbound_package = *thread_packages.get_object(outbound_slot);
                            FileLoadRequest file_load_request;
                            file_load_request.file = async_tga_load.file;
                            threading::CPodThreadMsg outbound_msg;
                            outbound_msg.async_slot = 0;
                            outbound_msg.payload.assign(file_load_request);
                            (void)outbound_package.post(outbound_msg);
                            break;
                        }
                        case (k_type_id_v<TgaSaveRequest>):
                        {
                            debug_utils::debug_output("Host: Recieved a TGA save request\n");

                            TgaSaveRequest tga_save_request;
                            (void)inbound_msg.payload.copy_to(tga_save_request);
                            async_tga_save.file = tga_save_request.file;
                            async_tga_save.options = *tga_save_request.options;
                            async_tga_save.rect_view = *tga_save_request.view;
                            const std::int32_t outbound_slot = thread_slots[static_cast<std::uint8_t>(EWorkerThreadID::bg_conditioning)];
                            CThreadPackage& outbound_package = *thread_packages.get_object(outbound_slot);
                            TgaEncodeRequest tga_encode_request;
                            tga_encode_request.view = &async_tga_save.rect_view;
                            tga_encode_request.options = &async_tga_save.options;
                            threading::CPodThreadMsg outbound_msg;
                            outbound_msg.async_slot = 0;
                            outbound_msg.payload.assign(tga_encode_request);
                            (void)outbound_package.post(outbound_msg);
                            break;
                        }
                        case (k_type_id_v<UnrecognisedMsg>):
                        {
                            UnrecognisedMsg unrecognised;
                            (void)inbound_msg.payload.copy_to(unrecognised);

                            debug_utils::debug_output("Host: Recieved an unrecognised message notification (%d)\n", unrecognised.msg_id);
                            break;
                        }
                        default:
                        {
                            debug_utils::debug_output("Host: Recieved an unrecognised message type (%d)\n", inbound_msg.payload.query_type_id());
                            break;
                        }
                    }
                }
                memory::CTypeless inbound_msg_owning;
                while (inbound_package.take_ownership(inbound_msg_owning))
                {
                    debug_utils::debug_output("Host: Recieved object ownership\n");

                    switch (inbound_msg_owning.query_type_id())
                    {
                        case (k_type_id_v<FileLoadResultOwning>):
                        {   //  for this test we know that this is in response to our own attempt to load the tga file
                            debug_utils::debug_output("Host: Took ownership of a loaded file buffer\n");

                            FileLoadResultOwning& result = *typeless_cast<FileLoadResultOwning>(inbound_msg_owning);
                            async_tga_load.buffer = std::move(result.buffer);
                            async_tga_load.view = async_tga_load.buffer.const_view();
                            const std::int32_t outbound_slot = thread_slots[static_cast<std::uint8_t>(EWorkerThreadID::bg_conditioning)];
                            CThreadPackage& outbound_package = *thread_packages.get_object(outbound_slot);
                            TgaDecodeRequest tga_decode_request;
                            tga_decode_request.view = &async_tga_load.view;
                            tga_decode_request.vflip = async_tga_load.vflip;
                            threading::CPodThreadMsg outbound_msg;
                            outbound_msg.async_slot = 0;
                            outbound_msg.payload.assign(tga_decode_request);
                            (void)outbound_package.post(outbound_msg);
                            break;
                        }
                        case (k_type_id_v<TgaEncodeResultOwning>):
                        {   //  for this test we know that this is in response to our own attempt to encode the tga file
                            debug_utils::debug_output("Host: Took ownership of an encoded TGA file buffer\n");

                            TgaEncodeResultOwning& result = *typeless_cast<TgaEncodeResultOwning>(inbound_msg_owning);
                            async_tga_save.buffer = std::move(result.buffer);
                            async_tga_save.view = async_tga_save.buffer.const_view();
                            const std::int32_t outbound_slot = thread_slots[static_cast<std::uint8_t>(EWorkerThreadID::bg_file_io)];
                            CThreadPackage& outbound_package = *thread_packages.get_object(outbound_slot);
                            FileSaveRequest file_save_request;
                            file_save_request.file = async_tga_save.file;
                            file_save_request.view = &async_tga_save.view;
                            threading::CPodThreadMsg outbound_msg;
                            outbound_msg.async_slot = 0;
                            outbound_msg.payload.assign(file_save_request);
                            (void)outbound_package.post(outbound_msg);
                            break;
                        }
                        case (k_type_id_v<TgaDecodeResultOwning>):
                        {   //  for this test we know that this is in response to our own attempt to decode the tga file
                            debug_utils::debug_output("Host: Took ownership of a decoded TGA image buffer\n");

                            TgaDecodeResultOwning& result = *typeless_cast<TgaDecodeResultOwning>(inbound_msg_owning);
                            async_tga_load.rect_buffer = std::move(result.buffer);
                            async_tga_load.rect_view = async_tga_load.rect_buffer.const_view();
                            async_tga_load.desc = result.desc;
                            const std::int32_t outbound_slot = thread_slots[static_cast<std::uint8_t>(EWorkerThreadID::application)];
                            CThreadPackage& outbound_package = *thread_packages.get_object(outbound_slot);
                            TgaLoadResult tga_load_result;
                            tga_load_result.view = &async_tga_load.rect_view;
                            tga_load_result.desc = async_tga_load.desc;
                            threading::CPodThreadMsg outbound_msg;
                            outbound_msg.async_slot = 0;
                            outbound_msg.payload.assign(tga_load_result);
                            (void)outbound_package.post(outbound_msg);
                            break;
                        }
                        default:
                        {
                            debug_utils::debug_output("Host: Took ownership of an unknown object (%d)\n", inbound_msg_owning.query_type_id());
                            break;
                        }
                    }
                }
            }
        }
    }

    for (std::int32_t thread_index = 2; thread_index >= 0; --thread_index)
    {
        const std::int32_t controller_slot = thread_slots[thread_index];
        if (controller_slot >= 0)
        {
            CThreadPackage& worker_package = *thread_packages.get_object(controller_slot);
            (void)worker_package.shutdown();
        }
    }
    return 0;
}
