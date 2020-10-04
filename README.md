# Closh

The functionality Closh is separed into the `seq` and `par` functions corresponding to sequential execution and parallel execution respectively.

First the `rep` function determines the parameters for running the process, such as the `count`, whether to run `seq` or `par` and the timeout value. The configuration for running the child process is stored in the `CloshState` struct.

 Both `seq` and `par` will run a program n time based on the input from the prompt.

For sequential exection, each iteration spanws one child process, and the process will hang until the child process is finished executing.

For parallel exection, a pool of processes will be forked upfront, and `exec` will be called right after the process is started so they execute in parallel. Parent process will wait for child proceses to finish after all iterations are finished.

When handling timeout, if we simply sleep on the parent process it can kill processes that run longer than the timeout, but for child processes that finish eariler we need a way to wake the sleeping parent process. To solve this we use an additional timer child process. The parent will still wait for child processes, in addtion to waiting for the timer process. When the timer process is done, it sets the `errno` flag to `EINTR`. The parent process will poll for `errno` while waiting and if it `errno` is equal to `EINTR` it will stop all child processes eariler.

## Team

Jinyang Yao: Inital programming.

Lucas Pozza: Documentation and programming fixes.