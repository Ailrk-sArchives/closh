# Closh

The repl is separated into `seq` and `par` two main functions, corresponds to sequential execution and parallel execution respectively.

First, both `seq` and `par` will run a program n time based on the input from the prompt, and they handle timeout differently.

For sequential exection, each iteration spanws one child proces, and it will not finish until the parent process the child process finish.

For parallel exection, a pool of processes will be forked upfront, and exec will be called right after the process is started so they execute in parallel. Parent process will wait for child proceses to finish after all iteration finished.

When handling timeout, if we use sleep it can timeout and kill process that run longer than the timeout, but for process execute faster than the timeout it needs a way to wake the sleeping parent process. To do that we use an additional timer child process, parent will still wait for other processes, and when timer process is done it will set errno to EINTR. The parent process polling on errno and if it find EINTR it will stop child processes eariler.

