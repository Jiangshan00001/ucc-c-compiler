// RUN: %archgen %s 'x86:andl $15, %%esp' 'x86_64:andq $15, %%rsp' -mpreferred-stack-boundary=4 -mstackrealign

// we should realign this regardless
// - could be called from 4-byte stack aligned code
f()
{
}
