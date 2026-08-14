# NewCore architecture - v1.0.3 phase gate

Pipeline đích:

Resolver -> Read-only Scanner -> Snapshot/State Store -> Observer -> FSM -> SafetyGuard -> ActionQueue(max=1) -> Proven MainThread Dispatcher -> Internal Action Engine -> Game -> ACK/Observer

v1.0.3 chỉ triển khai phase gate trước action:

Window discovery -> Hook bridge -> native validation -> IL2CPP metadata introspection -> report

Không có runtime_invoke và không có game mutation trong phase này.
