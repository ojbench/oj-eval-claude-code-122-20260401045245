#ifndef MEMO_HPP
#define MEMO_HPP

#include "event.h"
#include <iostream>

inline std::string CustomNotifyLateEvent::GetNotification(int n) const {
  // Call base class GetNotification first
  std::string base_msg = NotifyLateEvent::GetNotification(n);
  // Call the generator function
  std::string custom_msg = generator_(n);
  // Concatenate and return
  return base_msg + custom_msg;
}

// Node structure for linked list
struct EventNode {
  const Event* event;
  int notification_count;  // Track how many times this event has been notified
  EventNode* next;

  EventNode(const Event* e) : event(e), notification_count(0), next(nullptr) {}
};

class Memo {
 public:
  // Delete default constructor
  Memo() = delete;

  // Constructor with duration parameter
  Memo(int duration) : duration_(duration), current_time_(0) {
    // Allocate array of linked list heads for time slots 1 to duration
    time_lists_ = new EventNode*[duration_ + 1];
    for (int i = 0; i <= duration_; ++i) {
      time_lists_[i] = nullptr;
    }
  }

  // Destructor - must ensure no memory leaks
  ~Memo() {
    // Free all nodes in all linked lists
    for (int i = 0; i <= duration_; ++i) {
      EventNode* current = time_lists_[i];
      while (current != nullptr) {
        EventNode* next = current->next;
        delete current;
        current = next;
      }
    }
    delete[] time_lists_;
  }

  // Add an event to the memo
  void AddEvent(const Event* event) {
    if (event == nullptr) return;

    // Determine which time slot to add to based on event type
    int target_time = 0;

    // Try to determine event type using dynamic_cast
    const NormalEvent* normal_event = dynamic_cast<const NormalEvent*>(event);
    const NotifyBeforeEvent* notify_before = dynamic_cast<const NotifyBeforeEvent*>(event);
    const NotifyLateEvent* notify_late = dynamic_cast<const NotifyLateEvent*>(event);

    if (normal_event != nullptr) {
      // Normal event: add to deadline time
      target_time = event->GetDeadline();
    } else if (notify_before != nullptr) {
      // Notify before event: add to notify time (not deadline - notify_time!)
      // The notify_time parameter is the absolute time to notify
      target_time = notify_before->GetNotifyTime();
    } else if (notify_late != nullptr) {
      // Notify late event: add to deadline time
      target_time = event->GetDeadline();
    } else {
      // Default: add to deadline
      target_time = event->GetDeadline();
    }

    // Add to the end of the list at target_time
    if (target_time >= 1 && target_time <= duration_) {
      EventNode* new_node = new EventNode(event);

      if (time_lists_[target_time] == nullptr) {
        time_lists_[target_time] = new_node;
      } else {
        // Find the tail of the list
        EventNode* current = time_lists_[target_time];
        while (current->next != nullptr) {
          current = current->next;
        }
        current->next = new_node;
      }
    }
  }

  // Simulate time passing - process next hour
  void Tick() {
    current_time_++;

    if (current_time_ < 1 || current_time_ > duration_) {
      return;
    }

    // Process all events at current time
    EventNode* current = time_lists_[current_time_];
    EventNode* prev = nullptr;

    while (current != nullptr) {
      EventNode* next = current->next;
      const Event* event = current->event;

      if (event->IsComplete()) {
        // Event is complete, remove node
        if (prev == nullptr) {
          time_lists_[current_time_] = next;
        } else {
          prev->next = next;
        }
        delete current;
        current = next;
        continue;
      }

      // Event is not complete, need to notify
      // Determine event type and handle accordingly
      const NormalEvent* normal_event = dynamic_cast<const NormalEvent*>(event);
      const NotifyBeforeEvent* notify_before = dynamic_cast<const NotifyBeforeEvent*>(event);
      const NotifyLateEvent* notify_late = dynamic_cast<const NotifyLateEvent*>(event);

      if (normal_event != nullptr) {
        // Normal event: notify and remove
        std::cout << event->GetNotification(0) << std::endl;

        if (prev == nullptr) {
          time_lists_[current_time_] = next;
        } else {
          prev->next = next;
        }
        delete current;
        current = next;

      } else if (notify_before != nullptr) {
        // Check if this is notify time or deadline
        if (current_time_ == notify_before->GetNotifyTime()) {
          // This is notify time (first notification)
          std::cout << event->GetNotification(0) << std::endl;

          // Move to deadline time
          if (prev == nullptr) {
            time_lists_[current_time_] = next;
          } else {
            prev->next = next;
          }

          int deadline = event->GetDeadline();
          if (deadline <= duration_) {
            current->next = nullptr;
            current->notification_count = 1;

            // Add to end of deadline list
            if (time_lists_[deadline] == nullptr) {
              time_lists_[deadline] = current;
            } else {
              EventNode* tail = time_lists_[deadline];
              while (tail->next != nullptr) {
                tail = tail->next;
              }
              tail->next = current;
            }
          } else {
            delete current;
          }

          current = next;

        } else {
          // This is deadline time (second notification)
          std::cout << event->GetNotification(1) << std::endl;

          if (prev == nullptr) {
            time_lists_[current_time_] = next;
          } else {
            prev->next = next;
          }
          delete current;
          current = next;
        }

      } else if (notify_late != nullptr) {
        // Notify late event: notify and move to next check time
        std::cout << event->GetNotification(current->notification_count) << std::endl;

        // Move to next notification time
        int next_time = current_time_ + notify_late->GetFrequency();

        if (prev == nullptr) {
          time_lists_[current_time_] = next;
        } else {
          prev->next = next;
        }

        if (next_time <= duration_) {
          current->next = nullptr;
          current->notification_count++;

          // Add to end of next_time list
          if (time_lists_[next_time] == nullptr) {
            time_lists_[next_time] = current;
          } else {
            EventNode* tail = time_lists_[next_time];
            while (tail->next != nullptr) {
              tail = tail->next;
            }
            tail->next = current;
          }
        } else {
          delete current;
        }

        current = next;

      } else {
        // Unknown type, skip
        prev = current;
        current = next;
      }
    }
  }

 private:
  int duration_;
  int current_time_;
  EventNode** time_lists_;  // Array of linked list heads
};
#endif
