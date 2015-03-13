#pragma once

#include "../base/assert.hpp"
#include "../base/cancellable.hpp"
#include "../base/macros.hpp"

#include "../std/target_os.hpp"

#include "../std/noncopyable.hpp"
#include "../std/shared_ptr.hpp"
#include "../std/stdint.hpp"
#include "../std/thread.hpp"
#include "../std/unique_ptr.hpp"
#include "../std/utility.hpp"
#include "../std/vector.hpp"

#ifdef OMIM_OS_WINDOWS
#include "../std/windows.hpp"  // for DWORD
#endif

namespace threads
{
class IRoutine : public my::Cancellable
{
public:
  /// Perform the main task.
  virtual void Do() = 0;
};

/// A wrapper for system threads API.
///
/// Thread class manages lifetime of a running IRoutine and guarantees
/// that it will be possible to access the IRoutine after
/// Thread::Create()call until destruction of a Thread object.  In the
/// latter case, system thread will be responsible for deletion of a
/// IRoutine.
class Thread
{
  thread m_thread;
  shared_ptr<IRoutine> m_routine;

public:
  ~Thread();

  /// Run thread immediately.
  /// @param routine Routine that will be executed on m_thread and destroyed by
  ///                the current Thread instance or by the m_thread, if
  ///                it will be detached during the execution of routine.
  bool Create(unique_ptr<IRoutine> && routine);

  /// Calling the IRoutine::Cancel method, and Join'ing with the task
  /// execution.  After that, routine is deleted.
  void Cancel();

  /// Wait for thread ending.
  void Join();

  /// \return Pointer to the routine.
  IRoutine * GetRoutine();

  /// \return Pointer to the routine converted to T *.  When it's not
  ///         possible to convert routine to the T *, release version
  ///         returns nullptr, debug version fails.
  template <typename T>
  T * GetRoutineAs()
  {
    ASSERT(m_routine.get(), ("Routine is not set"));
    T * ptr = dynamic_cast<T *>(m_routine.get());
    ASSERT(ptr, ("Can't convert IRoutine* to", TO_STRING(T) "*"));
    return ptr;
  }
};

/// Simple threads container. Takes ownership for every added IRoutine.
class SimpleThreadPool : public noncopyable
{
  vector<unique_ptr<Thread>> m_pool;

public:
  SimpleThreadPool(size_t reserve = 0);

  void Add(unique_ptr<IRoutine> && routine);
  void Join();

  IRoutine * GetRoutine(size_t i) const;
};

/// Suspends the execution of the current thread until the time-out interval elapses.
/// @param[in] ms time-out interval in milliseconds
void Sleep(size_t ms);

typedef thread::id ThreadID;

ThreadID GetCurrentThreadID();

}  // namespace threads
