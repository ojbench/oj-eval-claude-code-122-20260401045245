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
  int event_type;  // 0=Normal, 1=NotifyBefore, 2=NotifyLate/CustomNotifyLate
  EventNode* next;

  EventNode(const Event* e, int type) : event(e), notification_count(0), event_type(type), next(nullptr) {}
};

class Memo {
 public:
  // Delete default constructor
  Memo() = delete;

  // Constructor with duration parameter
  Memo(int duration) : duration_(duration), current_time_(0) {
    // Allocate array of linked list heads for time slots 1 to duration
    time_lists_ = new EventNode*[duration_ + 1];
    time_tails_ = new EventNode*[duration_ + 1];
    for (int i = 0; i <= duration_; ++i) {
      time_lists_[i] = nullptr;
      time_tails_[i] = nullptr;
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
    delete[] time_tails_;
  }

  // Add an event to the memo
  void AddEvent(const Event* event) {
    if (event == nullptr) return;

    // Determine which time slot to add to based on event type
    int target_time = 0;
    int event_type = 0;

    // Try to determine event type using dynamic_cast (only once during add)
    const NormalEvent* normal_event = dynamic_cast<const NormalEvent*>(event);
    const NotifyBeforeEvent* notify_before = dynamic_cast<const NotifyBeforeEvent*>(event);
    const NotifyLateEvent* notify_late = dynamic_cast<const NotifyLateEvent*>(event);

    if (normal_event != nullptr) {
      // Normal event: add to deadline time
      target_time = event->GetDeadline();
      event_type = 0;
    } else if (notify_before != nullptr) {
      // Notify before event: add to notify time (not deadline - notify_time!)
      // The notify_time parameter is the absolute time to notify
      target_time = notify_before->GetNotifyTime();
      event_type = 1;
    } else if (notify_late != nullptr) {
      // Notify late event: add to deadline time
      target_time = event->GetDeadline();
      event_type = 2;
    } else {
      // Default: add to deadline
      target_time = event->GetDeadline();
      event_type = 0;
    }

    // Add to the end of the list at target_time using tail pointer
    if (target_time >= 1 && target_time <= duration_) {
      EventNode* new_node = new EventNode(event, event_type);

      if (time_lists_[target_time] == nullptr) {
        time_lists_[target_time] = new_node;
        time_tails_[target_time] = new_node;
      } else {
        time_tails_[target_time]->next = new_node;
        time_tails_[target_time] = new_node;
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
          if (next == nullptr) {
            time_tails_[current_time_] = nullptr;
          }
        } else {
          prev->next = next;
          if (next == nullptr) {
            time_tails_[current_time_] = prev;
          }
        }
        delete current;
        current = next;
        continue;
      }

      // Event is not complete, need to notify based on cached event type
      if (current->event_type == 0) {
        // Normal event: notify and remove
        std::cout << event->GetNotification(0) << std::endl;

        if (prev == nullptr) {
          time_lists_[current_time_] = next;
          if (next == nullptr) {
            time_tails_[current_time_] = nullptr;
          }
        } else {
          prev->next = next;
          if (next == nullptr) {
            time_tails_[current_time_] = prev;
          }
        }
        delete current;
        current = next;

      } else if (current->event_type == 1) {
        // NotifyBefore event
        const NotifyBeforeEvent* notify_before = static_cast<const NotifyBeforeEvent*>(event);

        if (current_time_ == notify_before->GetNotifyTime()) {
          // This is notify time (first notification)
          std::cout << event->GetNotification(0) << std::endl;

          // Remove from current list
          if (prev == nullptr) {
            time_lists_[current_time_] = next;
            if (next == nullptr) {
              time_tails_[current_time_] = nullptr;
            }
          } else {
            prev->next = next;
            if (next == nullptr) {
              time_tails_[current_time_] = prev;
            }
          }

          // Move to deadline time
          int deadline = event->GetDeadline();
          if (deadline <= duration_) {
            current->next = nullptr;
            current->notification_count = 1;

            // Add to end of deadline list using tail pointer
            if (time_lists_[deadline] == nullptr) {
              time_lists_[deadline] = current;
              time_tails_[deadline] = current;
            } else {
              time_tails_[deadline]->next = current;
              time_tails_[deadline] = current;
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
            if (next == nullptr) {
              time_tails_[current_time_] = nullptr;
            }
          } else {
            prev->next = next;
            if (next == nullptr) {
              time_tails_[current_time_] = prev;
            }
          }
          delete current;
          current = next;
        }

      } else if (current->event_type == 2) {
        // NotifyLate event: notify and move to next check time
        const NotifyLateEvent* notify_late = static_cast<const NotifyLateEvent*>(event);

        std::cout << event->GetNotification(current->notification_count) << std::endl;

        // Remove from current list
        if (prev == nullptr) {
          time_lists_[current_time_] = next;
          if (next == nullptr) {
            time_tails_[current_time_] = nullptr;
          }
        } else {
          prev->next = next;
          if (next == nullptr) {
            time_tails_[current_time_] = prev;
          }
        }

        // Move to next notification time
        int next_time = current_time_ + notify_late->GetFrequency();

        if (next_time <= duration_) {
          current->next = nullptr;
          current->notification_count++;

          // Add to end of next_time list using tail pointer
          if (time_lists_[next_time] == nullptr) {
            time_lists_[next_time] = current;
            time_tails_[next_time] = current;
          } else {
            time_tails_[next_time]->next = current;
            time_tails_[next_time] = current;
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
  EventNode** time_tails_;  // Array of linked list tails for O(1) append
};
#endif
