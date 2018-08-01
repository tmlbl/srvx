srvx
====

srvx is a microservice framework based on the Unix philosophy of decoupling
concerns through the file interface.

Instead of making HTTP calls or executing client code, services simply read and
write to files at certain paths to exchange messages through request/reply or
publish/subscribe patterns. Message transport is handled by zeroMQ.
