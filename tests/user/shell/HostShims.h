#ifndef TESTS_USER_SHELL_HOST_SHIMS_H
#define TESTS_USER_SHELL_HOST_SHIMS_H

// Test-settable results for the host `sys_taskctl()` shim defined in `CmdTop.cc`. The shell's real
// syscall wrappers are compiled out under `__DOORS_USER_HOST_TEST`.

namespace hostshim {

void setTaskctlKillResult(int result);
void setTaskctlDetailResult(int result);

} // namespace hostshim

#endif // TESTS_USER_SHELL_HOST_SHIMS_H
