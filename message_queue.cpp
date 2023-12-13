#include <cassert>
#include <ctime>
#include <pthread.h>
#include <semaphore.h>
#include "guard.h"
#include "message.h"
#include "message_queue.h"

MessageQueue::MessageQueue() {
  // TODO: initialize the mutex and the semaphore
  pthread_mutex_init(&m_lock, nullptr);
  sem_init(&m_avail, 0, 0);
}

MessageQueue::~MessageQueue() {
  // TODO: destroy the mutex and the semaphore
  pthread_mutex_destroy(&m_lock);
  sem_destroy(&m_avail);
}

void MessageQueue::enqueue(Message *msg) {
  // TODO: put the specified message on the queue
  Guard g(m_lock);

  m_messages.push_back(msg);
  // be sure to notify any thread waiting for a message to be
  // available by calling sem_post
  sem_post(&m_avail);
}

Message *MessageQueue::dequeue() {
  // TODO: call sem_wait to wait up to 1 second for a message
  //       to be available, return nullptr if no message is available
  // TODO: remove the next message from the queue, return it

  Message *msg = nullptr;
  sem_wait(&m_avail); /* wait for item */

  Guard g(m_lock);
  msg = m_messages.front();
  m_messages.pop_front();
  
  return msg;
}

 