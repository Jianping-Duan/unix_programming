UNIX Programming cases based on FreeBSD environment
--------------------------------------------------------------------------------

The project takes the FreeBSD operating system as an example to describe
programming based on the unix environment in the form of programming cases.

For the consideration of readability, the submodules in the project are divided
by several system calls of unix, and each .c source file in the project will
generate a corresponding executable file after compilation. 

The system call function or library function used in the source file can be used
to view the introduction of the function using man(2) or man(3).

Source RoadMap:
--------------------------------------------------------------------------------
|   Directory     |         Description         |
|-----------------|-----------------------------|
| file            | Several operations of files on unix, including Attribute,
IO, Directory and Lock |
| signals         | signal processing in unix operation system |
