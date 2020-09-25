# Closh

The repl is separated into `seq` and `par` two main functions, corresponds to sequential execution and parallel execution respectively.

First, both `seq` and `par` will run a program n time based on the input from the prompt.

For sequential exection, each iteration spanws one child process, and it will hang until the child process just spawned finished.

For parallel exection, a pool of processes will be forked upfront, and exec will be called right after the process is started so they execute in parallel. Parent process will wait for child proceses to finish after all iteration finished.

When handling timeout, if we simply sleep on parent it can kill processes that run longer than the timeout, but for process finish eariler we need a way to wake the sleeping parent process. Instead we use an additional timer child process, parent will still wait for child processes, in addtion to that when timer process is done it will set errno to EINTR. The parent process polling on errno while waiting and if it find EINTR it will stop child processes eariler.
