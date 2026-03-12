# DESCRIPTION
* Tasktree is a to-do app that arranges tasks in a tree structure
* The idea is to have subtasks of subtasks of subtasks.
* It is currently being rewritten in C++: see my tasktree-cpp repository.

# COMPILING
* make sure cmake, a c compiler, and vcpkg are installed
* run 'cmake --preset default'
* run 'cmake --build build'

# BASIC USAGE
* Tasks are currently passed as a path like so: *"task-name1/task-name2/..."*
* Use *"new [parent task]"* to make a new task. If no parent task is given, it will make a root task.
* use "complete [task]" to mark it as completed
* Use *"remove [task]"* to remove it.
* Use *"list"* to list tasks.
