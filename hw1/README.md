# Secured API Call

## Introduction

This project implement the **Library Hijacking** and **GOT Table Rewriting**.
Before rewriting got table, it need to **parse elf** of execution file to get got offset.

This project aims to monitor the process of execution (print log file).

## Program Launcher

Program **launcher** execute a command and load the **sandbox<span/>.so** using LD_PRELOAD. The launcher executes the command and passes the required environment variables to an invoked process. The environment variables include:

* **SANDBOX_CONFIG**: The path of the configuration file for sandbox<span/>.so
* **LOGGER_FD**: the file descriptor (fd) for logging messages.

## Sandbox
Implement the **sandbox<span/>.so** that supports the following features.

* Implement a **__libc_start_main** to hijack the process’s entry point.

* In the **__libc_start_main**, you should perform the necessary initializations and then call the real **__libc_start_main**.

* Your **sandbox<span/>.so** cannot have a function with the same function name listed in the API function list. To hijack an API function, you must perform GOT hijacking in the libc_start_main of **sandbox<span/>.so**. The functions you have to hijack are listed in the API Function List section.

## API Function List

All functions listed below should be logged to the file descriptor (fd) passed by the environment variable **LOGGER_FD**.

* **open**
    Allow a user to set the file access blacklist so that files listed in the blacklist cannot be opened. If a file is in the blacklist, return -1 and set errno to EACCES. Note that for handling symbolic linked files, your implementation has to follow the links before performing the checks.

* **read**
    Your implementation must log all content into a file. The log file should be named in the format {pid}-{fd}-read.log and be created at read function on this fd be called first time. (If an fd is used more than one time in a process, keep logging the content into the same log file.)

    Furthermore, your implementation has to allow a user to filter the read content based on a keyword blacklist. The filter should be active for all read operations. If the filter detects a matched keyword in a read content, close the fd and return -1 with an errno setting to EIO. Do not log the content if it is filtered.

* **write**
    Your implementation must log all content into a file. The log file should be named in the format {pid}-{fd}-write.log and be created at write function on this fd be called first time. (If an fd is used more than one time in a process, keep logging the content into the same log file.)

* **connect**
    Allow a user to block connection setup to specific IP addresses and PORT numbers. If the IP and PORT is blocked, return -1 and set errno to ECONNREFUSED.

* **getaddrinfo**
    Allow a user to block specific host name resolution requests. If a host is blocked, return EAI_NONAME.

* **system**
    Commands invoked by system function should also be hijacked and monitored by your sandbox. Note that you cannot invoke the launcher again in your implementation. The correct and incorrect relationship for the invoked process should look like the example below.

## Configuration File Format

The **configuration file** is a text file containing blocked content for each API function. For each API, the general form is as follows.

```typescript
BEGIN <API>-blacklist
rule1
rule2
...
END <API>-blacklist
```

A sample configuration is given below.

```typescript
BEGIN open-blacklist
/etc/passwd
/etc/shadow
END open-blacklist

BEGIN read-blacklist
-----BEGIN CERTIFICATE-----
END read-blacklist

BEGIN connect-blacklist
www.nycu.edu.tw:4433
google.com:80
END connect-blacklist

BEGIN getaddrinfo-blacklist
www.ym.edu.tw
www.nctu.edu.tw
END getaddrinfo-blacklist
```

## Examples

All the running examples use the same configuration given below.

```typescript
BEGIN open-blacklist
/etc/passwd
/etc/group
END open-blacklist

BEGIN read-blacklist
-----BEGIN CERTIFICATE-----
END read-blacklist

BEGIN connect-blacklist
www.nycu.edu.tw:443
google.com:80
END connect-blacklist

BEGIN getaddrinfo-blacklist
www.ym.edu.tw
www.nctu.edu.tw
google.com
END getaddrinfo-blacklist
```

### Example 1

* command: ./launcher ./sandbox.so config.txt cat /etc/passwd
* output:

```typescript
[logger] open("/etc/passwd", 0, 0) = -1
cat: /etc/passwd: Permission denied
```

### Example 2

* command: ./launcher ./sandbox.so config.txt cat /etc/hosts
* output:

```typescript
[logger] open("/etc/hosts", 0, 0) = 5
[logger] read(5, 0x7fb7b2db2000, 131072) = 177
127.0.0.1       localhost
::1     localhost ip6-localhost ip6-loopback
fe00::0 ip6-localnet
ff00::0 ip6-mcastprefix
ff02::1 ip6-allnodes
ff02::2 ip6-allrouters
192.168.208.2   00fd90b988c7
[logger] write(1, 0x7fb7b2db2000, 177) = 177
[logger] read(5, 0x7fb7b2db2000, 131072) = 0
```

### Example 3

* command: ./launcher ./sandbox.so config.txt cat /etc/ssl/certs/Amazon_Root_CA_1.pem
* output:

```typescript
[logger] open("/usr/share/ca-certificates/mozilla/Amazon_Root_CA_1.crt", 0, 0) = 5
[logger] read(5, 0x7f6a9c486000, 131072) = -1
cat: /etc/ssl/certs/Amazon_Root_CA_1.pem: Input/output error
cat: /etc/ssl/certs/Amazon_Root_CA_1.pem: Bad file descriptor
```
