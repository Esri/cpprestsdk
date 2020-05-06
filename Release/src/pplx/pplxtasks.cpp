/***
 * Copyright (C) Microsoft. All rights reserved.
 * Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
 *
 * =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *
 * Parallel Patterns Library implementation (common code across platforms)
 *
 * For the latest on this and related APIs, please see: https://github.com/Microsoft/cpprestsdk
 *
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 ****/

#include "stdafx.h"

#if !defined(_WIN32) || CPPREST_FORCE_PPLX

#include "pplx/pplxtasks.h"

// Disable false alarm code analyze warning
#if defined(_MSC_VER)
#pragma warning (disable : 26165 26110)
#endif

namespace pplx
{

  bool _pplx_cdecl is_task_cancellation_requested()
  {
    return ::pplx::details::_TaskCollection_t::_Is_cancellation_requested();
  }

  __declspec(noreturn) void _pplx_cdecl cancel_current_task() 
  { 
    throw task_canceled(); 
  }

namespace details
{
  _TaskCreationCallstack::_TaskCreationCallstack()
  {
    _M_SingleFrame = nullptr;
  }

  _TaskCreationCallstack _TaskCreationCallstack::_CaptureSingleFrameCallstack(void *_SingleFrame)
  {
    _TaskCreationCallstack _csc;
    _csc._M_SingleFrame = _SingleFrame;
    return _csc;
  }

  _TaskCreationCallstack _TaskCreationCallstack::_CaptureMultiFramesCallstack(size_t _CaptureFrames)
  {
    _TaskCreationCallstack _csc;
    _csc._M_frames.resize(_CaptureFrames);
    // skip 2 frames to make sure callstack starts from user code
    _csc._M_frames.resize(::pplx::details::platform::CaptureCallstack(&_csc._M_frames[0], 2, _CaptureFrames));
    return _csc;
  }

  _TaskProcThunk::_TaskProcThunk(const std::function<void()> & _Callback) :
    _M_func(_Callback)
  {
  }

  void _pplx_cdecl _TaskProcThunk::_Bridge(void *_PData)
  {
    _TaskProcThunk *_PThunk = reinterpret_cast<_TaskProcThunk *>(_PData);
    _Holder _ThunkHolder(_PThunk);
    _PThunk->_M_func();
  }

  _TaskProcThunk::_Holder::_Holder(_TaskProcThunk * _PThunk) : _M_pThunk(_PThunk)
  {
  }

  _TaskProcThunk::_Holder::~_Holder()
  {
    delete _M_pThunk;
  }

  void _ScheduleFuncWithAutoInline(const std::function<void()> & _Func, _TaskInliningMode_t _InliningMode)
  {
    _TaskCollection_t::_RunTask(&_TaskProcThunk::_Bridge, new _TaskProcThunk(_Func), _InliningMode);
  }

#if defined (__cplusplus_winrt)
  _ContextCallback _ContextCallback::_CaptureCurrent()
  {
    _ContextCallback _Context;
    _Context._Capture();
    return _Context;
  }

  _ContextCallback::~_ContextCallback()
  {
    _Reset();
  }

  _ContextCallback::_ContextCallback(bool _DeferCapture /*= false*/)
  {
    if (_DeferCapture)
    {
      _M_context._M_captureMethod = _S_captureDeferred;
    }
    else
    {
      _M_context._M_pContextCallback = nullptr;
    }
  }

  _ContextCallback::_ContextCallback(const _ContextCallback& _Src)
  {
    _Assign(_Src._M_context._M_pContextCallback);
  }

  _ContextCallback::_ContextCallback(_ContextCallback&& _Src)
  {
    _M_context._M_pContextCallback = _Src._M_context._M_pContextCallback;
    _Src._M_context._M_pContextCallback = nullptr;
  }

  bool _ContextCallback::_HasCapturedContext() const
  {
    _ASSERTE(_M_context._M_captureMethod != _S_captureDeferred);
    return (_M_context._M_pContextCallback != nullptr);
  }

  void _ContextCallback::_Resolve(bool _CaptureCurrent)
  {
    if (_M_context._M_captureMethod == _S_captureDeferred)
    {
      _M_context._M_pContextCallback = nullptr;

      if (_CaptureCurrent)
      {
        if (_IsCurrentOriginSTA())
        {
          _Capture();
        }
#if _UITHREADCTXT_SUPPORT
        else
        {
          // This method will fail if not called from the UI thread.
          HRESULT _Hr = CaptureUiThreadContext(&_M_context._M_pContextCallback);
          if (FAILED(_Hr))
          {
            _M_context._M_pContextCallback = nullptr;
          }
        }
#endif /* _UITHREADCTXT_SUPPORT */
      }
    }
  }

  void _ContextCallback::_Capture()
  {
    HRESULT _Hr =
      CoGetObjectContext(IID_IContextCallback, reinterpret_cast<void**>(&_M_context._M_pContextCallback));
    if (FAILED(_Hr))
    {
      _M_context._M_pContextCallback = nullptr;
    }
  }

  _ContextCallback& _ContextCallback::operator=(const _ContextCallback& _Src)
  {
    if (this != &_Src)
    {
      _Reset();
      _Assign(_Src._M_context._M_pContextCallback);
    }
    return *this;
  }

  _ContextCallback& _ContextCallback::operator=(_ContextCallback&& _Src)
  {
    if (this != &_Src)
    {
      _M_context._M_pContextCallback = _Src._M_context._M_pContextCallback;
      _Src._M_context._M_pContextCallback = nullptr;
    }
    return *this;
  }

  void _ContextCallback::_CallInContext(_CallbackFunction _Func) const
  {
    if (!_HasCapturedContext())
    {
      _Func();
    }
    else
    {
      ComCallData callData;
      ZeroMemory(&callData, sizeof(callData));
      callData.pUserDefined = reinterpret_cast<void*>(&_Func);

      HRESULT _Hr = _M_context._M_pContextCallback->ContextCallback(
        &_Bridge, &callData, IID_ICallbackWithNoReentrancyToApplicationSTA, 5, nullptr);
      if (FAILED(_Hr))
      {
        throw ::Platform::Exception::CreateException(_Hr);
      }
    }
  }

  void _ContextCallback::_Reset()
  {
    if (_M_context._M_captureMethod != _S_captureDeferred && _M_context._M_pContextCallback != nullptr)
    {
      _M_context._M_pContextCallback->Release();
    }
  }

  bool _ContextCallback::operator==(const _ContextCallback& _Rhs) const
  {
    return (_M_context._M_pContextCallback == _Rhs._M_context._M_pContextCallback);
  }

  bool _ContextCallback::operator!=(const _ContextCallback& _Rhs) const
  {
    return !(operator==(_Rhs));
  }

  void _ContextCallback::_Assign(IContextCallback *_PContextCallback)
  {
    _M_context._M_pContextCallback = _PContextCallback;
    if (_M_context._M_captureMethod != _S_captureDeferred && _M_context._M_pContextCallback != nullptr)
    {
      _M_context._M_pContextCallback->AddRef();
    }
  }

  HRESULT __stdcall _ContextCallback::_Bridge(ComCallData *_PParam)
  {
    _CallbackFunction* pFunc = reinterpret_cast<_CallbackFunction*>(_PParam->pUserDefined);
    (*pFunc)();
    return S_OK;
  }

  bool _ContextCallback::_IsCurrentOriginSTA()
  {
    APTTYPE _AptType;
    APTTYPEQUALIFIER _AptTypeQualifier;

    HRESULT hr = CoGetApartmentType(&_AptType, &_AptTypeQualifier);
    if (SUCCEEDED(hr))
    {
      // We determine the origin of a task continuation by looking at where .then is called, so we can tell
      // whether to need to marshal the continuation back to the originating apartment. If an STA thread is in
      // executing in a neutral apartment when it schedules a continuation, we will not marshal continuations back
      // to the STA, since variables used within a neutral apartment are expected to be apartment neutral.
      switch (_AptType)
      {
      case APTTYPE_MAINSTA:
      case APTTYPE_STA: return true;
      default: break;
      }
    }
    return false;
  }

#else
  _ContextCallback _ContextCallback::_CaptureCurrent()
  {
    return _ContextCallback();
  }

  _ContextCallback::_ContextCallback(bool /*= false*/)
  {
  }

  _ContextCallback::_ContextCallback(const _ContextCallback&)
  {
  }

  _ContextCallback::_ContextCallback(_ContextCallback&&)
  {
  }

  _ContextCallback& _ContextCallback::operator=(const _ContextCallback&)
  {
    return *this;
  }

  _ContextCallback& _ContextCallback::operator=(_ContextCallback&&)
  {
    return *this;
  }

  bool _ContextCallback::_HasCapturedContext() const
  {
    return false;
  }

  void _ContextCallback::_Resolve(bool) const
  {
  }


  void _ContextCallback::_CallInContext(_CallbackFunction _Func) const
  {
    _Func();
  }


  bool _ContextCallback::operator==(const _ContextCallback&) const
  {
    return true;
  }


  bool _ContextCallback::operator!=(const _ContextCallback&) const
  {
    return false;
  }
#endif /* defined (__cplusplus_winrt) */

  void _ExceptionHolder::ReportUnhandledError()
  {
#if _MSC_VER >= 1800 && defined(__cplusplus_winrt)
    if (_M_winRTException != nullptr)
    {
      ::Platform::Details::ReportUnhandledError(_M_winRTException);
    }
#endif /* defined (__cplusplus_winrt) */
  }

  _ExceptionHolder::_ExceptionHolder(const std::exception_ptr& _E, const _TaskCreationCallstack &_stackTrace) : _M_exceptionObserved(0)
    , _M_stdException(_E)
    , _M_stackTrace(_stackTrace)
#if defined(__cplusplus_winrt)
    , _M_winRTException(nullptr)
#endif /* defined (__cplusplus_winrt) */
  {
  }

#if defined (__cplusplus_winrt)
  _ExceptionHolder::_ExceptionHolder(::Platform::Exception ^ _E, const _TaskCreationCallstack& _stackTrace)
    : _M_exceptionObserved(0), _M_winRTException(_E), _M_stackTrace(_stackTrace)
  {
  }
#endif  /* defined (__cplusplus_winrt) */

  _ExceptionHolder::~_ExceptionHolder()
  {
    if (_M_exceptionObserved == 0)
    {
      // If you are trapped here, it means an exception thrown in task chain didn't get handled.
      // Please add task-based continuation to handle all exceptions coming from tasks.
      // this->_M_stackTrace keeps the creation callstack of the task generates this exception.
      _REPORT_PPLTASK_UNOBSERVED_EXCEPTION();
    }
  }


  void _ExceptionHolder::_RethrowUserException()
  {
    if (_M_exceptionObserved == 0)
    {
      atomic_exchange(_M_exceptionObserved, 1l);
    }

#if defined(__cplusplus_winrt)
    if (_M_winRTException != nullptr)
    {
      throw _M_winRTException;
    }
#endif /* defined (__cplusplus_winrt) */
    std::rethrow_exception(_M_stdException);
  }

  void _Internal_task_options::_set_creation_callstack(const _TaskCreationCallstack &_callstack)
  {
    _M_hasPresetCreationCallstack = true;
    _M_presetCreationCallstack = _callstack;
  }

  _Internal_task_options::_Internal_task_options()
  {
    _M_hasPresetCreationCallstack = false;
  }

  _Internal_task_options & _get_internal_task_options(task_options &options)
  {
    return options._M_InternalTaskOptions;
  }

  const _Internal_task_options & _get_internal_task_options(const task_options &options)
  {
    return options._M_InternalTaskOptions;
  }

  _ContinuationTaskHandleBase::_ContinuationTaskHandleBase() : _M_next(nullptr)
    , _M_continuationContext(task_continuation_context::use_default())
    , _M_isTaskBasedContinuation(false)
    , _M_inliningMode(details::_NoInline)
  {
  }

  _ContinuationTaskHandleBase::~_ContinuationTaskHandleBase()
  {
  }

#if PPLX_TASK_ASYNC_LOGGING
  bool _IsCausalitySupported()
  {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    if (_isCausalitySupported == 0)
    {
      long _causality = 1;
      OSVERSIONINFOEX _osvi = {};
      _osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

      // The Causality is supported on Windows version higher than Windows 8
      _osvi.dwMajorVersion = 6;
      _osvi.dwMinorVersion = 3;

      DWORDLONG _conditionMask = 0;
      VER_SET_CONDITION(_conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
      VER_SET_CONDITION(_conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

      if (::VerifyVersionInfo(&_osvi, VER_MAJORVERSION | VER_MINORVERSION, _conditionMask))
      {
        _causality = 2;
      }

      _isCausalitySupported = _causality;
      return _causality == 2;
    }

    return _isCausalitySupported == 2 ? true : false;
#else
    return true;
#endif
  }

  void _TaskEventLogger::_LogScheduleTask(bool _isContinuation)
  {
    if (details::_IsCausalitySupported())
    {
      ::Windows::Foundation::Diagnostics::AsyncCausalityTracer::TraceOperationCreation(
        ::Windows::Foundation::Diagnostics::CausalityTraceLevel::Required,
        ::Windows::Foundation::Diagnostics::CausalitySource::Library,
        _PPLTaskCausalityPlatformID,
        reinterpret_cast<unsigned long long>(_M_task),
        _isContinuation ? "pplx::PPLTask::ScheduleContinuationTask" : "pplx::PPLTask::ScheduleTask",
        0);
      _M_scheduled = true;
    }
  }

  void _TaskEventLogger::_LogCancelTask()
  {
    if (details::_IsCausalitySupported())
    {
      ::Windows::Foundation::Diagnostics::AsyncCausalityTracer::TraceOperationRelation(
        ::Windows::Foundation::Diagnostics::CausalityTraceLevel::Important,
        ::Windows::Foundation::Diagnostics::CausalitySource::Library,
        _PPLTaskCausalityPlatformID,
        reinterpret_cast<unsigned long long>(_M_task),
        ::Windows::Foundation::Diagnostics::CausalityRelation::Cancel);
    }
  }

  void _TaskEventLogger::_LogTaskExecutionStarted()
  {
  }

  void _TaskEventLogger::_LogTaskCompleted()
  {
    if (_M_scheduled)
    {
      ::Windows::Foundation::AsyncStatus _State;
      if (_M_task->_IsCompleted())
        _State = ::Windows::Foundation::AsyncStatus::Completed;
      else if (_M_task->_HasUserException())
        _State = ::Windows::Foundation::AsyncStatus::Error;
      else
        _State = ::Windows::Foundation::AsyncStatus::Canceled;

      if (details::_IsCausalitySupported())
      {
        ::Windows::Foundation::Diagnostics::AsyncCausalityTracer::TraceOperationCompletion(
          ::Windows::Foundation::Diagnostics::CausalityTraceLevel::Required,
          ::Windows::Foundation::Diagnostics::CausalitySource::Library,
          _PPLTaskCausalityPlatformID,
          reinterpret_cast<unsigned long long>(_M_task),
          _State);
      }
    }
  }

  void _TaskEventLogger::_LogTaskExecutionCompleted()
  {
    if (_M_taskPostEventStarted && details::_IsCausalitySupported())
    {
      ::Windows::Foundation::Diagnostics::AsyncCausalityTracer::TraceSynchronousWorkCompletion(
        ::Windows::Foundation::Diagnostics::CausalityTraceLevel::Required,
        ::Windows::Foundation::Diagnostics::CausalitySource::Library,
        ::Windows::Foundation::Diagnostics::CausalitySynchronousWork::CompletionNotification);
    }
  }

  void _TaskEventLogger::_LogWorkItemStarted()
  {
    if (details::_IsCausalitySupported())
    {
      ::Windows::Foundation::Diagnostics::AsyncCausalityTracer::TraceSynchronousWorkStart(
        ::Windows::Foundation::Diagnostics::CausalityTraceLevel::Required,
        ::Windows::Foundation::Diagnostics::CausalitySource::Library,
        _PPLTaskCausalityPlatformID,
        reinterpret_cast<unsigned long long>(_M_task),
        ::Windows::Foundation::Diagnostics::CausalitySynchronousWork::Execution);
    }
  }

  void _TaskEventLogger::_LogWorkItemCompleted()
  {
    if (details::_IsCausalitySupported())
    {
      ::Windows::Foundation::Diagnostics::AsyncCausalityTracer::TraceSynchronousWorkCompletion(
        ::Windows::Foundation::Diagnostics::CausalityTraceLevel::Required,
        ::Windows::Foundation::Diagnostics::CausalitySource::Library,
        ::Windows::Foundation::Diagnostics::CausalitySynchronousWork::Execution);

      ::Windows::Foundation::Diagnostics::AsyncCausalityTracer::TraceSynchronousWorkStart(
        ::Windows::Foundation::Diagnostics::CausalityTraceLevel::Required,
        ::Windows::Foundation::Diagnostics::CausalitySource::Library,
        _PPLTaskCausalityPlatformID,
        reinterpret_cast<unsigned long long>(_M_task),
        ::Windows::Foundation::Diagnostics::CausalitySynchronousWork::CompletionNotification);
      _M_taskPostEventStarted = true;
    }
  }

  _TaskEventLogger::_TaskEventLogger(_Task_impl_base *_task) : _M_task(_task)
  {
    _M_scheduled = false;
    _M_taskPostEventStarted = false;
  }

  _TaskWorkItemRAIILogger::_TaskWorkItemRAIILogger(_TaskEventLogger &_taskHandleLogger) : _M_logger(_taskHandleLogger)
  {
    _M_logger._LogWorkItemStarted();
  }

  _TaskWorkItemRAIILogger::~_TaskWorkItemRAIILogger()
  {
    _M_logger._LogWorkItemCompleted();
  }
#else
  void _LogCancelTask(_Task_impl_base *)
  {
  }
  
  void _TaskEventLogger::_LogScheduleTask(bool)
  {
  }

  void _TaskEventLogger::_LogCancelTask()
  {
  }

  void _TaskEventLogger::_LogWorkItemStarted()
  {
  }

  void _TaskEventLogger::_LogWorkItemCompleted()
  {
  }

  void _TaskEventLogger::_LogTaskExecutionStarted()
  {
  }

  void _TaskEventLogger::_LogTaskExecutionCompleted()
  {
  }

  void _TaskEventLogger::_LogTaskCompleted()
  {
  }

  _TaskEventLogger::_TaskEventLogger(_Task_impl_base *)
  {
  }

  _TaskWorkItemRAIILogger::_TaskWorkItemRAIILogger(_TaskEventLogger &)
  {
  }
#endif // PPLX_TASK_ASYNC_LOGGING

  _Task_impl_base::_Task_impl_base(_CancellationTokenState * _PTokenState, scheduler_ptr _Scheduler_arg) : _M_TaskState(_Created)
    , _M_fFromAsync(false)
    , _M_fUnwrappedTask(false)
    , _M_pRegistration(nullptr)
    , _M_Continuations(nullptr)
    , _M_TaskCollection(_Scheduler_arg)
    , _M_taskEventLogger(this)
  {
    // Set cancellation token
    _M_pTokenState = _PTokenState;
    _ASSERTE(_M_pTokenState != nullptr);
    if (_M_pTokenState != _CancellationTokenState::_None()) _M_pTokenState->_Reference();
  }

  _Task_impl_base::~_Task_impl_base()
  {
    _ASSERTE(_M_pTokenState != nullptr);
    if (_M_pTokenState != _CancellationTokenState::_None())
    {
      _M_pTokenState->_Release();
    }
  }

  pplx::task_status _Task_impl_base::_Wait()
  {
    bool _DoWait = true;

#if defined(__cplusplus_winrt)
    if (_IsNonBlockingThread())
    {
      // In order to prevent Windows Runtime STA threads from blocking the UI, calling task.wait() task.get() is
      // illegal if task has not been completed.
      if (!_IsCompleted() && !_IsCanceled())
      {
        throw invalid_operation("Illegal to wait on a task in a Windows Runtime STA");
      }
      else
      {
        // Task Continuations are 'scheduled' *inside* the chore that is executing on the ancestors's task
        // group. If a continuation needs to be marshaled to a different apartment, instead of scheduling, we
        // make a synchronous cross apartment COM call to execute the continuation. If it then happens to do
        // something which waits on the ancestor (say it calls .get(), which task based continuations are wont
        // to do), waiting on the task group results in on the chore that is making this synchronous callback,
        // which causes a deadlock. To avoid this, we test the state ancestor's event , and we will NOT wait on
        // if it has finished execution (which means now we are on the inline synchronous callback).
        _DoWait = false;
      }
  }
#endif /* defined (__cplusplus_winrt) */
    if (_DoWait)
    {
      // If this task was created from a Windows Runtime async operation, do not attempt to inline it. The
      // async operation will take place on a thread in the appropriate apartment Simply wait for the completed
      // event to be set.
      if (_M_fFromAsync)
      {
        _M_TaskCollection._Wait();
      }
      else
      {
        // Wait on the task collection to complete. The task collection is guaranteed to still be
        // valid since the task must be still within scope so that the _Task_impl_base destructor
        // has not yet been called. This call to _Wait potentially inlines execution of work.
        try
        {
          // Invoking wait on a task collection resets the state of the task collection. This means that
          // if the task collection itself were canceled, or had encountered an exception, only the first
          // call to wait will receive this status. However, both cancellation and exceptions flowing through
          // tasks set state in the task impl itself.

          // When it returns canceled, either work chore or the cancel thread should already have set task's
          // state properly -- canceled state or completed state (because there was no interruption point).
          // For tasks with unwrapped tasks, we should not change the state of current task, since the
          // unwrapped task are still running.
          _M_TaskCollection._RunAndWait();
        }
        catch (details::_Interruption_exception&)
        {
          // The _TaskCollection will never be an interruption point since it has a none token.
          _ASSERTE(false);
        }
        catch (task_canceled&)
        {
          // task_canceled is a special exception thrown by cancel_current_task. The spec states that
          // cancel_current_task must be called from code that is executed within the task (throwing it from
          // parallel work created by and waited upon by the task is acceptable). We can safely assume that
          // the task wrapper _PPLTaskHandle::operator() has seen the exception and canceled the task. Swallow
          // the exception here.
          _ASSERTE(_IsCanceled());
        }
#if defined(__cplusplus_winrt)
        catch (::Platform::Exception ^ _E)
        {
          // Its possible the task body hasn't seen the exception, if so we need to cancel with exception
          // here.
          if (!_HasUserException())
          {
            _CancelWithException(_E);
        }
          // Rethrow will mark the exception as observed.
          _M_exceptionHolder->_RethrowUserException();
      }
#endif /* defined (__cplusplus_winrt) */
        catch (...)
        {
          // Its possible the task body hasn't seen the exception, if so we need to cancel with exception
          // here.
          if (!_HasUserException())
          {
            _CancelWithException(std::current_exception());
          }
          // Rethrow will mark the exception as observed.
          _M_exceptionHolder->_RethrowUserException();
        }

        // If the lambda body for this task (executed or waited upon in _RunAndWait above) happened to return a
        // task which is to be unwrapped and plumbed to the output of this task, we must not only wait on the
        // lambda body, we must wait on the **INNER** body. It is in theory possible that we could inline such
        // if we plumb a series of things through; however, this takes the tact of simply waiting upon the
        // completion signal.
        if (_M_fUnwrappedTask)
        {
          _M_TaskCollection._Wait();
        }
    }
        }

    if (_HasUserException())
    {
      _M_exceptionHolder->_RethrowUserException();
    }
    else if (_IsCanceled())
    {
      return canceled;
    }
    _ASSERTE(_IsCompleted());
    return completed;
    }

  bool _Task_impl_base::_Cancel(bool _SynchronousCancel)
  {
    // Send in a dummy value for exception. It is not used when the first parameter is false.
    return _CancelAndRunContinuations(_SynchronousCancel, false, false, _M_exceptionHolder);
  }

  bool _Task_impl_base::_CancelWithExceptionHolder(const std::shared_ptr<_ExceptionHolder>& _ExHolder, bool _PropagatedFromAncestor)
  {
    // This task was canceled because an ancestor task encountered an exception.
    return _CancelAndRunContinuations(true, true, _PropagatedFromAncestor, _ExHolder);
  }

#if defined (__cplusplus_winrt)
  bool _Task_impl_base::_CancelWithException(::Platform::Exception^ _Exception)
  {
    // This task was canceled because the task body encountered an exception.
    _ASSERTE(!_HasUserException());
    return _CancelAndRunContinuations(
      true, true, false, std::make_shared<_ExceptionHolder>(_Exception, _GetTaskCreationCallstack()));
    }
#endif  /* defined (__cplusplus_winrt) */

  bool _Task_impl_base::_CancelWithException(const std::exception_ptr& _Exception)
  {
    // This task was canceled because the task body encountered an exception.
    _ASSERTE(!_HasUserException());
    return _CancelAndRunContinuations(
      true, true, false, std::make_shared<_ExceptionHolder>(_Exception, _GetTaskCreationCallstack()));
  }

  void _Task_impl_base::_RegisterCancellation(std::weak_ptr<_Task_impl_base> _WeakPtr)
  {
    _ASSERTE(details::_CancellationTokenState::_IsValid(_M_pTokenState));

    auto _CancellationCallback = [_WeakPtr]() {
      // Taking ownership of the task prevents dead lock during destruction
      // if the destructor waits for the cancellations to be finished
      auto _task = _WeakPtr.lock();
      if (_task != nullptr) _task->_Cancel(false);
    };

    _M_pRegistration =
      new details::_CancellationTokenCallback<decltype(_CancellationCallback)>(_CancellationCallback);
    _M_pTokenState->_RegisterCallback(_M_pRegistration);
  }

  void _Task_impl_base::_DeregisterCancellation()
  {
    if (_M_pRegistration != nullptr)
    {
      _M_pTokenState->_DeregisterCallback(_M_pRegistration);
      _M_pRegistration->_Release();
      _M_pRegistration = nullptr;
    }
  }

  bool _Task_impl_base::_IsCreated()
  {
    return (_M_TaskState == _Created);
  }

  bool _Task_impl_base::_IsStarted()
  {
    return (_M_TaskState == _Started);
  }

  bool _Task_impl_base::_IsPendingCancel()
  {
    return (_M_TaskState == _PendingCancel);
  }

  bool _Task_impl_base::_IsCompleted()
  {
    return (_M_TaskState == _Completed);
  }

  bool _Task_impl_base::_IsCanceled()
  {
    return (_M_TaskState == _Canceled);
  }

  bool _Task_impl_base::_HasUserException()
  {
    return static_cast<bool>(_M_exceptionHolder);
  }

  const std::shared_ptr<_ExceptionHolder>& _Task_impl_base::_GetExceptionHolder()
  {
    _ASSERTE(_HasUserException());
    return _M_exceptionHolder;
  }

  bool _Task_impl_base::_IsApartmentAware()
  {
    return _M_fFromAsync;
  }

  void _Task_impl_base::_SetAsync(bool _Async /*= true*/)
  {
    _M_fFromAsync = _Async;
  }

  _TaskCreationCallstack _Task_impl_base::_GetTaskCreationCallstack()
  {
    return _M_pTaskCreationCallstack;
  }

  void _Task_impl_base::_SetTaskCreationCallstack(const _TaskCreationCallstack &_Callstack)
  {
    _M_pTaskCreationCallstack = _Callstack;
  }

  void _Task_impl_base::_ScheduleTask(_UnrealizedChore_t * _PTaskHandle, _TaskInliningMode_t _InliningMode)
  {
    try
    {
      _M_TaskCollection._ScheduleTask(_PTaskHandle, _InliningMode);
    }
    catch (const task_canceled&)
    {
      // task_canceled is a special exception thrown by cancel_current_task. The spec states that
      // cancel_current_task must be called from code that is executed within the task (throwing it from parallel
      // work created by and waited upon by the task is acceptable). We can safely assume that the task wrapper
      // _PPLTaskHandle::operator() has seen the exception and canceled the task. Swallow the exception here.
      _ASSERTE(_IsCanceled());
    }
    catch (const _Interruption_exception&)
    {
      // The _TaskCollection will never be an interruption point since it has a none token.
      _ASSERTE(false);
    }
    catch (...)
    {
      // The exception could have come from two places:
      //   1. From the chore body, so it already should have been caught and canceled.
      //      In this case swallow the exception.
      //   2. From trying to actually schedule the task on the scheduler.
      //      In this case cancel the task with the current exception, otherwise the
      //      task will never be signaled leading to deadlock when waiting on the task.
      if (!_HasUserException())
      {
        _CancelWithException(std::current_exception());
      }
    }
  }

  void _Task_impl_base::_RunContinuation(_ContinuationTaskHandleBase * _PTaskHandle)
  {
    _Task_ptr_base _ImplBase = _PTaskHandle->_GetTaskImplBase();
    if (_IsCanceled() && !_PTaskHandle->_M_isTaskBasedContinuation)
    {
      if (_HasUserException())
      {
        // If the ancestor encountered an exception, transfer the exception to the continuation
        // This traverses down the tree to propagate the exception.
        _ImplBase->_CancelWithExceptionHolder(_GetExceptionHolder(), true);
      }
      else
      {
        // If the ancestor was canceled, then your own execution should be canceled.
        // This traverses down the tree to cancel it.
        _ImplBase->_Cancel(true);
      }
    }
    else
    {
      // This can only run when the ancestor has completed or it's a task based continuation that fires when a
      // task is canceled (with or without a user exception).
      _ASSERTE(_IsCompleted() || _PTaskHandle->_M_isTaskBasedContinuation);
      _ASSERTE(!_ImplBase->_IsCanceled());
      return _ImplBase->_ScheduleContinuationTask(_PTaskHandle);
    }

    // If the handle is not scheduled, we need to manually delete it.
    delete _PTaskHandle;
  }

  void _Task_impl_base::_ScheduleContinuationTask(_ContinuationTaskHandleBase * _PTaskHandle)
  {
    _M_taskEventLogger._LogScheduleTask(true);
    // Ensure that the continuation runs in proper context (this might be on a Concurrency Runtime thread or in a
    // different Windows Runtime apartment)
    if (_PTaskHandle->_M_continuationContext._HasCapturedContext())
    {
      // For those continuations need to be scheduled inside captured context, we will try to apply automatic
      // inlining to their inline modes, if they haven't been specified as _ForceInline yet. This change will
      // encourage those continuations to be executed inline so that reduce the cost of marshaling. For normal
      // continuations we won't do any change here, and their inline policies are completely decided by ._ThenImpl
      // method.
      if (_PTaskHandle->_M_inliningMode != details::_ForceInline)
      {
        _PTaskHandle->_M_inliningMode = details::_DefaultAutoInline;
      }
      _ScheduleFuncWithAutoInline(
        [_PTaskHandle]() {
        // Note that we cannot directly capture "this" pointer, instead, we should use _TaskImplPtr, a
        // shared_ptr to the _Task_impl_base. Because "this" pointer will be invalid as soon as _PTaskHandle
        // get deleted. _PTaskHandle will be deleted after being scheduled.
        auto _TaskImplPtr = _PTaskHandle->_GetTaskImplBase();
        if (details::_ContextCallback::_CaptureCurrent() == _PTaskHandle->_M_continuationContext)
        {
          _TaskImplPtr->_ScheduleTask(_PTaskHandle, details::_ForceInline);
        }
        else
        {
          //
          // It's entirely possible that the attempt to marshal the call into a differing context will
          // fail. In this case, we need to handle the exception and mark the continuation as canceled
          // with the appropriate exception. There is one slight hitch to this:
          //
          // NOTE: COM's legacy behavior is to swallow SEH exceptions and marshal them back as HRESULTS.
          // This will in effect turn an SEH into a C++ exception that gets tagged on the task. One
          // unfortunate result of this is that various pieces of the task infrastructure will not be in a
          // valid state after this in /EHsc (due to the lack of destructors running, etc...).
          //
          try
          {
            // Dev10 compiler needs this!
            auto _PTaskHandle1 = _PTaskHandle;
            _PTaskHandle->_M_continuationContext._CallInContext([_PTaskHandle1, _TaskImplPtr]() {
              _TaskImplPtr->_ScheduleTask(_PTaskHandle1, details::_ForceInline);
            });
          }
#if defined(__cplusplus_winrt)
          catch (::Platform::Exception ^ _E)
          {
            _TaskImplPtr->_CancelWithException(_E);
          }
#endif /* defined (__cplusplus_winrt) */
          catch (...)
          {
            _TaskImplPtr->_CancelWithException(std::current_exception());
          }
          }
        },
        _PTaskHandle->_M_inliningMode);
      }
    else
    {
      _ScheduleTask(_PTaskHandle, _PTaskHandle->_M_inliningMode);
    }
    }

  void _Task_impl_base::_ScheduleContinuation(_ContinuationTaskHandleBase * _PTaskHandle)
  {
    enum
    {
      _Nothing,
      _Schedule,
      _Cancel,
      _CancelWithException
    } _Do = _Nothing;

    // If the task has canceled, cancel the continuation. If the task has completed, execute the continuation right
    // away. Otherwise, add it to the list of pending continuations
    {
      ::pplx::extensibility::scoped_critical_section_t _LockHolder(_M_ContinuationsCritSec);
      if (_IsCompleted() || (_IsCanceled() && _PTaskHandle->_M_isTaskBasedContinuation))
      {
        _Do = _Schedule;
      }
      else if (_IsCanceled())
      {
        if (_HasUserException())
        {
          _Do = _CancelWithException;
        }
        else
        {
          _Do = _Cancel;
        }
      }
      else
      {
        // chain itself on the continuation chain.
        _PTaskHandle->_M_next = _M_Continuations;
        _M_Continuations = _PTaskHandle;
      }
    }

    // Cancellation and execution of continuations should be performed after releasing the lock. Continuations off
    // of async tasks may execute inline.
    switch (_Do)
    {
    case _Schedule:
    {
      _PTaskHandle->_GetTaskImplBase()->_ScheduleContinuationTask(_PTaskHandle);
      break;
    }
    case _Cancel:
    {
      // If the ancestor was canceled, then your own execution should be canceled.
      // This traverses down the tree to cancel it.
      _PTaskHandle->_GetTaskImplBase()->_Cancel(true);

      delete _PTaskHandle;
      break;
    }
    case _CancelWithException:
    {
      // If the ancestor encountered an exception, transfer the exception to the continuation
      // This traverses down the tree to propagate the exception.
      _PTaskHandle->_GetTaskImplBase()->_CancelWithExceptionHolder(_GetExceptionHolder(), true);

      delete _PTaskHandle;
      break;
    }
    case _Nothing:
    default:
      // In this case, we have inserted continuation to continuation chain,
      // nothing more need to be done, just leave.
      break;
    }
  }

  void _Task_impl_base::_RunTaskContinuations()
  {
    // The link list can no longer be modified at this point,
    // since all following up continuations will be scheduled by themselves.
    _ContinuationList _Cur = _M_Continuations, _Next;
    _M_Continuations = nullptr;
    while (_Cur)
    {
      // Current node might be deleted after running,
      // so we must fetch the next first.
      _Next = _Cur->_M_next;
      _RunContinuation(_Cur);
      _Cur = _Next;
    }
  }

#if defined (__cplusplus_winrt)
  bool _Task_impl_base::_IsNonBlockingThread()
  {
    APTTYPE _AptType;
    APTTYPEQUALIFIER _AptTypeQualifier;

    HRESULT hr = CoGetApartmentType(&_AptType, &_AptTypeQualifier);
    //
    // If it failed, it's not a Windows Runtime/COM initialized thread. This is not a failure.
    //
    if (SUCCEEDED(hr))
    {
      switch (_AptType)
      {
      case APTTYPE_STA:
      case APTTYPE_MAINSTA: return true; break;
      case APTTYPE_NA:
        switch (_AptTypeQualifier)
        {
          // A thread executing in a neutral apartment is either STA or MTA. To find out if this thread is
          // allowed to wait, we check the app qualifier. If it is an STA thread executing in a neutral
          // apartment, waiting is illegal, because the thread is responsible for pumping messages and
          // waiting on a task could take the thread out of circulation for a while.
        case APTTYPEQUALIFIER_NA_ON_STA:
        case APTTYPEQUALIFIER_NA_ON_MAINSTA: return true; break;
        }
        break;
      }
    }

#if _UITHREADCTXT_SUPPORT
    // This method is used to throw an exception in _Wait() if called within STA.  We
    // want the same behavior if _Wait is called on the UI thread.
    if (SUCCEEDED(CaptureUiThreadContext(nullptr)))
    {
      return true;
    }
#endif /* _UITHREADCTXT_SUPPORT */

    return false;
  }
#endif  /* defined (__cplusplus_winrt) */

  pplx::scheduler_ptr _Task_impl_base::_GetScheduler() const
  {
    return _M_TaskCollection._GetScheduler();
  }

  std::function<_Unit_type(void)> _MakeVoidToUnitFunc(const std::function<void(void)>& _Func)
  {
    return [=]() -> _Unit_type {
      _Func();
      return _Unit_type();
    };
  }

  std::function<_Unit_type(_Unit_type)> _MakeUnitToUnitFunc(const std::function<void(void)>& _Func)
  {
    return [=](_Unit_type) -> _Unit_type {
      _Func();
      return _Unit_type();
    };
  }

#if defined (__cplusplus_winrt)
  task<void> _To_task_helper(Windows::Foundation::IAsyncAction^ op)
  {
    return task<void>(op);
  }
#endif  /* defined (__cplusplus_winrt) */
} // namespace details

task_continuation_context task_continuation_context::use_default()
{
#if defined(__cplusplus_winrt)
  // The callback context is created with the context set to CaptureDeferred and resolved when it is used in
  // .then()
  return task_continuation_context(
    true); // sets it to deferred, is resolved in the constructor of _ContinuationTaskHandle
#else  /* defined (__cplusplus_winrt) */
  return task_continuation_context();
#endif /* defined (__cplusplus_winrt) */
}

#if defined (__cplusplus_winrt)
task_continuation_context task_continuation_context::use_arbitrary()
{
  task_continuation_context _Arbitrary(true);
  _Arbitrary._Resolve(false);
  return _Arbitrary;
}

task_continuation_context task_continuation_context::use_current()
{
  task_continuation_context _Current(true);
  _Current._Resolve(true);
  return _Current;
}
#endif  /* defined (__cplusplus_winrt) */

task_continuation_context::task_continuation_context(bool _DeferCapture /*= false*/) : details::_ContextCallback(_DeferCapture)
{
}

task_options::task_options() : _M_Scheduler(get_ambient_scheduler())
, _M_CancellationToken(cancellation_token::none())
, _M_ContinuationContext(task_continuation_context::use_default())
, _M_HasCancellationToken(false)
, _M_HasScheduler(false)
{
}

task_options::task_options(cancellation_token _Token) : _M_Scheduler(get_ambient_scheduler())
, _M_CancellationToken(_Token)
, _M_ContinuationContext(task_continuation_context::use_default())
, _M_HasCancellationToken(true)
, _M_HasScheduler(false)
{
}

task_options::task_options(task_continuation_context _ContinuationContext) : _M_Scheduler(get_ambient_scheduler())
, _M_CancellationToken(cancellation_token::none())
, _M_ContinuationContext(_ContinuationContext)
, _M_HasCancellationToken(false)
, _M_HasScheduler(false)
{
}

task_options::task_options(cancellation_token _Token, task_continuation_context _ContinuationContext) : _M_Scheduler(get_ambient_scheduler())
, _M_CancellationToken(_Token)
, _M_ContinuationContext(_ContinuationContext)
, _M_HasCancellationToken(false)
, _M_HasScheduler(false)
{
}

task_options::task_options(scheduler_interface& _Scheduler) : _M_Scheduler(&_Scheduler)
, _M_CancellationToken(cancellation_token::none())
, _M_ContinuationContext(task_continuation_context::use_default())
, _M_HasCancellationToken(false)
, _M_HasScheduler(true)
{
}

task_options::task_options(scheduler_ptr _Scheduler) : _M_Scheduler(std::move(_Scheduler))
, _M_CancellationToken(cancellation_token::none())
, _M_ContinuationContext(task_continuation_context::use_default())
, _M_HasCancellationToken(false)
, _M_HasScheduler(true)
{
}

task_options::task_options(const task_options& _TaskOptions) : _M_Scheduler(_TaskOptions.get_scheduler())
, _M_CancellationToken(_TaskOptions.get_cancellation_token())
, _M_ContinuationContext(_TaskOptions.get_continuation_context())
, _M_HasCancellationToken(_TaskOptions.has_cancellation_token())
, _M_HasScheduler(_TaskOptions.has_scheduler())
{
}

void task_options::set_cancellation_token(cancellation_token _Token)
{
  _M_CancellationToken = _Token;
  _M_HasCancellationToken = true;
}

void task_options::set_continuation_context(task_continuation_context _ContinuationContext)
{
  _M_ContinuationContext = _ContinuationContext;
}

bool task_options::has_cancellation_token() const
{
  return _M_HasCancellationToken;
}

cancellation_token task_options::get_cancellation_token() const
{
  return _M_CancellationToken;
}

task_continuation_context task_options::get_continuation_context() const
{
  return _M_ContinuationContext;
}

bool task_options::has_scheduler() const
{
  return _M_HasScheduler;
}

scheduler_ptr task_options::get_scheduler() const
{
  return _M_Scheduler;
}
} // namespace pplx

#endif
