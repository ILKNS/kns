## Flow Group
Adding or removing a thread, flow is migrated for load balancing. The migration is serialized and only one such process at any time for simplicity. 
The hrtimer is originally used here.
- Signals A to migrate `fgs` to B. A mark is used to distinguish it from normal ones, moving the packets from this group to remote one and stops 
all the timers belong to this group.
- Then A changed the RSS index to direct the flow to B. Thus the packet from thread A are lead to B.
- After that, when B receives the first packet from A, B signals A, remove the tag and add the timer to flow group.
![](https://github.com/ILKNS/kns/blob/master/document/fg.png)
