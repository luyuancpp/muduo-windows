// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H
#define MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"

#include <boost/circular_buffer.hpp>
#include <assert.h>

namespace muduo
{

template<typename T>
class BoundedBlockingQueue : noncopyable
{
 public:
  explicit BoundedBlockingQueue(int maxSize)
    : mutex_(),
#ifdef __muduo_asynchronization__
      notEmpty_(mutex_),
      notFull_(mutex_),
#else
      notEmpty_(),
      notFull_(),
#endif
      queue_(maxSize)
  {
  }

  void put(const T& x)
  {
    MutexLockGuard lock(mutex_);
    while (queue_.full())
    {
#ifdef __muduo_asynchronization__
      notFull_.wait();
#else
      notFull_.wait(lock);
#endif
    }
    assert(!queue_.full());
    queue_.push_back(x);
    notEmpty_.notify();
  }

  void put(T&& x)
  {
    MutexLockGuard lock(mutex_);
    while (queue_.full())
    {
#ifdef __muduo_asynchronization__
      notFull_.wait();
#else
      notFull_.wait(lock);
#endif
    }
    assert(!queue_.full());
    queue_.push_back(std::move(x));
    notEmpty_.notify();
  }

  T take()
  {
    MutexLockGuard lock(mutex_);
    while (queue_.empty())
    {
#ifdef __muduo_asynchronization__
      notEmpty_.wait();
#else
      notEmpty_.wait(lock);
#endif
    }
    assert(!queue_.empty());
    T front(std::move(queue_.front()));
    queue_.pop_front();
    notFull_.notify();
    return front;
  }

  bool empty() const
  {
    MutexLockGuard lock(mutex_);
    return queue_.empty();
  }

  bool full() const
  {
    MutexLockGuard lock(mutex_);
    return queue_.full();
  }

  size_t size() const
  {
    MutexLockGuard lock(mutex_);
    return queue_.size();
  }

  size_t capacity() const
  {
    MutexLockGuard lock(mutex_);
    return queue_.capacity();
  }

 private:
  mutable MutexLock          mutex_;
  Condition                  notEmpty_ GUARDED_BY(mutex_);
  Condition                  notFull_ GUARDED_BY(mutex_);
  boost::circular_buffer<T>  queue_ GUARDED_BY(mutex_);
};

}  // namespace muduo

#endif  // MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H
