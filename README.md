
# Bomberstudent - M1 RES

## Table of Contents

1. [RES Game Project](#res-game-project)

2. [How to use](#how-to-use)

    1. [Prerequisites](#prerequisites)

    2. [Plugins](#plugins)

    3. [V-0.1.311023](#v-01311023)

3. [Scripts](#scripts)

    1. [Stress testing the TCP server](#stress-testing-the-server)

    2. [Setup Script](#setup-script)

## RES Game Project

***Contributors**: Hadrien Belleville & Ugo Courche*

***Date of first commit**: October 31, 2023*

> This Git repository only contains the source code of the C server for the Bomberstudent game project.

### How to use

> This section contains the methods of how to use this code at current state.
>
> How to read the versions name: `V-X.Y.Z`
>
> + `X`: This indicates the major version. It is incremented for significant changes that may introduce backward-incompatible features.
> + `Y`: This is the minor version. It is typically incremented for smaller, backward-compatible changes or feature additions.
> + `Z`: This represents the patch version. It is used for bug fixes or small improvements that are backward-compatible. In our case this will always be the date of the first commit of the patch version.

#### Prerequisites

> Since version [V-0.2.151123](#v-02151123), there are no dependencies to install before building and using the project.

Before compiling and running this project, make sure to set up the necessary environment dependencies.

+ Update your environment:

```bash
sudo apt-get update
```

#### Plugins

##### Todo Tree

> from Gruntfuggly, [zaonce.com](https://zaonce.com).

We use this extension available on the marketplace of VSCode to organize the workspace with TODO tags.

Simple documentation on how to use TODO comment and configure in the workspace  can be found [here](https://thomasventurini.com/articles/the-best-way-to-work-with-todos-in-vscode/).

Also interesting if we want to use icons, the list of all supported icons can be found [here](https://microsoft.github.io/vscode-codicons/dist/codicon.html).

#### V-0.1.311023

To test the code at this current state follow these steps:

+ Open a Terminal for the server and navigate to the Makefile directory. Run:

```bash
    make && ./main
```

+ In separate Terminal windows for each client, establish a connection to the server and initiate the stream flow:

```bash
    nc localhost 42069
```

+ Feel free to send messages to the server from any connected client terminal. Don't forget to start with a polite greeting - say 'hello' first!

#### V-0.2.151123

This new version of the project is similar to [V-0.1.311023](#v-01311023), and the commands to run and test the code are the same. We only changed our JSON file management library to use `cJSON.h` instead of `jansson.h`.

### Scripts

> This section covers all scripts that can be used for testing purposes or anything that we may find usefull...

#### Stress Testing the server

The following script can be used if needed to create multiple instances of clients with connections in the server. It can be used for debugging and testing purposes, especially to stress testing the TCP server.

```bash
#!/bin/bash

# Check if an argument is provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <number_of_iterationsX>"
    exit 1
fi

# Retrieve the number of iterations from the first argument
n=$1

# Loop and execute command in a child process
for ((i=0; i<n; i++)); do
    (
        nc localhost 42069
    ) &
done

# Wait for all child process to end
wait
```

#### Setup Script

> This setup script is no longer necessary since we don't use the `jansson.h` library anymore.

This Bash script automates the setup process for compiling and running a project that requires specific environment dependencies.

```bash
#!/bin/bash

# Check if the script is being run with superuser rights
if [ "$EUID" -ne 0 ]
  then echo "Please run with superuser rights (sudo)"
  exit
fi

# Update the environment
apt-get update

# Install autoreconf, libtool, and automake
apt-get install autoconf libtool automake

# Navigate to the jansson-master directory
cd jansson-master

# Autoreconf (if necessary)
autoreconf -i

# Configure, make, and install Jansson
./configure
make
make install
```
