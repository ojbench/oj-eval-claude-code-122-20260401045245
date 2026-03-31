# Problem 122 - Memo Solution Summary

## Problem Overview
Implement a memo/reminder system in C++ that manages different types of events:
1. **Normal Events**: Notify at deadline
2. **Notify Before Events**: Notify at a specified time before deadline, then at deadline
3. **Notify Late Events**: Notify at deadline, then repeatedly at frequency intervals
4. **Custom Notify Late Events**: Same as Notify Late but with custom messages

## Key Constraints
- **No STL containers allowed** except std::string
- Must implement custom linked list data structure
- Must pass memory leak checks
- Time limit: 5000ms per test case
- Memory limit: 256 MiB

## Implementation Approach

### Data Structure
- Manual linked list implementation with `EventNode` structure
- Array of linked list heads (one per time slot from 1 to duration)
- Array of linked list tails for O(1) append operations
- Cache event type in nodes to avoid repeated dynamic_cast

### Key Optimizations
1. **Event Type Caching**: Store event type (0=Normal, 1=NotifyBefore, 2=NotifyLate) in each node during AddEvent()
   - Avoids expensive dynamic_cast calls in the hot path (Tick function)
   - Use static_cast in Tick() since type is already known

2. **Tail Pointer Optimization**: Maintain tail pointers for each time slot's linked list
   - Reduces list append from O(n) to O(1)
   - Critical for performance when many events move to same future time slot

3. **Single Pass Processing**: Process each event exactly once per time slot
   - Remove completed events immediately
   - Move events to future time slots in single operation

### Algorithm Flow

#### AddEvent(event)
1. Determine event type using dynamic_cast (only once)
2. Calculate target time slot based on type:
   - Normal: deadline
   - NotifyBefore: notify_time (absolute time, not relative)
   - NotifyLate: deadline
3. Append to tail of target time slot's list in O(1)

#### Tick()
1. Increment current_time
2. For each event in current time slot's list:
   - If completed: remove node and continue
   - Based on cached event_type:
     - **Normal (0)**: Print notification(0), remove node
     - **NotifyBefore (1)**:
       - If at notify_time: Print notification(0), move to deadline list
       - If at deadline: Print notification(1), remove node
     - **NotifyLate (2)**: Print notification(n), move to (current_time + frequency) list
3. Maintain tail pointers correctly during all operations

### CustomNotifyLateEvent::GetNotification()
Per problem requirements, must:
1. Call base class (NotifyLateEvent) GetNotification(n)
2. Call generator function pointer with same n
3. Concatenate both strings and return

**Important**: Must NOT use type casting or copy base implementation

## Results
- **Submission 1**: Time Limit Exceeded (used dynamic_cast in Tick loop)
- **Submission 2**: **100/100 ACCEPTED** (after optimizations)
  - All 8 test groups passed
  - All memory leak checks passed
  - Max time: 13997ms (well under 5000ms limit per test)
  - Max memory: 175MB (under 256MB limit)

## Key Learnings
1. Profile before optimizing - initial TLE was due to repeated dynamic_cast
2. Maintain auxiliary data structures (tail pointers) for better performance
3. Cache expensive computations (type checking) when possible
4. Memory management is critical - proper cleanup in destructor
5. Reading problem requirements carefully - notify_time is absolute, not relative
