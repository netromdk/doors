// C++ linkage matches the `abort()` in libc++/stdlib/abort.cc.
__attribute__((__noreturn__)) void abort();

int main()
{
  abort();
}
