## genpass

[![Build Status](https://travis-ci.org/chilicuil/genpass.png?branch=master)](https://travis-ci.org/chilicuil/genpass)

A stateless password generator.

<p align="center">
<img src="https://raw.githubusercontent.com/chilicuil/genpass/master/genpass.gif" alt="genpass"/>
</p>

## Quick start

### Ubuntu based systems (LTS versions)

1. Set up the minos archive:

   ```
   $ sudo add-apt-repository ppa:minos-archive/main
   ```

2. Install:

   ```
   $ sudo apt-get update && sudo apt-get install genpass
   ```

### Other Linux distributions

1. Compile from source

   ```
   $ make
   ```

2. Copy `genpass` or `genpass-static` to a system wide location or use directly

   ```
   $ cp genpass genpass-static /usr/local/bin/
   ```

Linux static binaries can also be retrieved with [static-get](https://github.com/minos-org/minos-static), for x86|amd64 platforms

1. Fetch static binaries

   ```
   $ sh <(wget -qO- s.minos.io/s) -x genpass
   ```

2. Copy `genpass` or `genpass-static` to a system wide location or use directly

   ```
   $ cp genpass-*/genpass genpass-*/genpass-static /usr/local/bin/
   ```

## Usage

First time usage:

    $ genpass
    Name: Guy Mann
    Site: github.com
    Master password: passwd #it won't be shown
    4c%7hZ5w]MZUB6RRPCJ&?wKTFtd[6Oj.P.02d+kIs

This will prompt you for your name, site and master password. The first time it's executed it will take a relative long time (a couple of minutes) to get back. It'll create a cache key and will save it to `~/.genpass-cache`, then it will combine it with the master password and the site string to generate the final password. The cache key file should be guarded with moderate caution. If it gets leaked possible attackers may have an easier time guessing your master password (although it still will be considerably harder than average brute force attacks).

General use

    $ genpass [options] [site]

Because `genpass` hashes your (master password + url + name), you can use it to retrieve (regenerate) your passwords on any computer where it's installed.

It's recommended to defined cost, length and other parameters explicitly, default values will change between versions as computers get updated on CPU/RAM.

Default values for version `2016.05.04` are:

Parameter             | Value
--------------------- | -------------
Cache cost (Scrypt N) | 2^20
Cost       (Scrypt N) | 2^14
Scrypt r              | 8 bits
Scrypt p              | 16 bits
Key length            | 32 bytes, 256 bits
Encoding              | z85

Past default values are listed in the [defaults.md](https://github.com/chilicuil/genpass/blob/master/defaults.md) file.

Also, you can pass the configuration file (.ini file extension) using `--config` option. Even if some parameters are ommited from configuration file, then it will either take input from command line or default value will be assigned. Refer to [genpass-example.ini](https://github.com/chilicuil/genpass/blob/master/config/genpass-example.ini) file

## Scheme

The [scheme](https://www.cs.utexas.edu/~bwaters/publications/papers/www2005.pdf) uses two levels of hash computations (although with the -1 parameter it can use only one). The first level is executed once when a user begins to use a new machine for the first time. This computation is parameterized to take a relatively long time (around 60 seconds on this implementation) and its result are cached for future password calculations by the same user. The next level is used to compute site-specific passwords. It takes as input the calculation produced from the first level as well as the name of the site or account for which the user is interested, the computation time is parameterized to be fast (around .1 seconds in our implementation).

Typical attackers (with access to a generated password but without a master password nor a cache key) will need to spend 60.1 seconds on average per try and will have little room for parallelization, legitimate users on the other hand will require 0.1s after the initial cache key is calculated. This way the scheme strives for the best balance between security and usability.

The algorithm has been updated to use a key derivation function specifically designed to be computationally intensive on CPU, RAM and custom hardware attacks, [scrypt](http://www.tarsnap.com/scrypt/scrypt.pdf). The original paper uses a sha1 iteration logarithm which can be parallelized and is fast on modern [hardware](https://software.intel.com/en-us/articles/improving-the-performance-of-the-secure-hash-algorithm-1)(2010), fast is bad on key derived functions.
