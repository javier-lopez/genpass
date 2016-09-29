//genpass: stateless password generator
//usage: genpass [option]... [site]

//example: genpass github.com
//Name: Joe Foobar
//Master password:
//4E1FQYCc.6M18R$rjl5]EO5FwkS>fVEw-KaNwROer

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/resource.h>

#include "poison/poison.h"
#include "arg_parser/arg_parser.h"
#include "readpass/readpass.h"

#include "libscrypt/b10.h"
#include "libscrypt/b64.h"
#include "libscrypt/z85.h"
#include "libscrypt/skey.h"
#include "libscrypt/libscrypt.h"
#include "base91/base91.h"

#define VERSION "2016.05.04"

//libscrypt/libscrypt.h:57
#undef  SCRYPT_HASH_LEN
#define SCRYPT_HASH_LEN       32 //or 256 bits
#define SCRYPT_HASH_LEN_MAX 1024
#define SCRYPT_HASH_LEN_MIN    8
#define SCRYPT_CACHE_COST     20
#define SCRYPT_COST           14
#define SCRYPT_SAFE_N         30
#define SCRYPT_r               8
#define SCRYPT_SAFE_r       9999
#define SCRYPT_p              16
#define SCRYPT_SAFE_p      99999

#define DEFAULT_ENCODING  "z85"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

void version(void) {
    const char *version_message=VERSION;
    fprintf(stdout, "%s\n", version_message);
    exit (EXIT_SUCCESS);
}

void usage(int status) {
    const char *usage_message="Usage: genpass [option]... [site]\n\
    \b\b\b\bStateless password generator.\
      \n\
      \n  -n, --name \"Full Name\"    name\
      \n  -p, --password \"Secret\"   master password\
      \n  -s, --site \"site.tld\"     site login\
      \n  -r, --registration-mode   ask twice for master password\
      \n  -f, --file FILE           use|write cache key from|to FILE\
      \n  -l, --key-length 8-1024   key length in bytes, \""TOSTRING(SCRYPT_HASH_LEN)"\" by default\
      \n  -C, --cache-cost 1-30     cpu/memory cost for cache key, \""TOSTRING(SCRYPT_CACHE_COST)"\" by default\
      \n  -c, --cost 1-30           cpu/memory cost for final key, \""TOSTRING(SCRYPT_COST)"\" by default\
      \n      --scrypt-r 1-9999     block size, \""TOSTRING(SCRYPT_r)"\" by default (advanced)\
      \n      --scrypt-p 1-99999    parallelization, \""TOSTRING(SCRYPT_p)"\" by default (advanced)\
      \n  -N, --dry-run             perform a trial run with no changes made\
      \n  -e, --encoding ENCODING   password encoding output, \""DEFAULT_ENCODING"\" by default\
      \n                              ENCODING: dec|hex|base64|z85|skey\
      \n  -1, --single              use single function derivation\
      \n  -v, --verbose             verbose mode\
      \n  -V, --version             show version and exit\
      \n  -h, --help                show this help message and exit\n";
    if (status != EXIT_SUCCESS) fprintf(stderr, "%s", usage_message);
    else fprintf(stdout, "%s", usage_message);
    exit(status);
}

void die(const char * const msg, const int errcode, const char help) {
    if (msg && msg[0]) {
        fprintf(stderr, "genpass: %s\n", msg);
        if (help) usage(EXIT_FAILURE);
        else exit (EXIT_FAILURE);
    }
    if (help) usage(EXIT_FAILURE);
}

void verbose(const char * const msg, const int verbose_lvl) {
    if (verbose_lvl > 0)
        if (msg && msg[0])
            fprintf(stderr, "[verbose] %s\n", msg);
}

uint64_t _pow(const unsigned int a, const unsigned int b) {
    int i, pow = 1;
    for (i = 0; i < b; i++)
        pow *= a;
    return pow;
}

const char * optname(const int code, const struct ap_Option options[]) {
    static char buf[2] = "?";
    int i;
    if (code != 0) {
        for (i = 0; options[i].code; ++i) {
            if (code == options[i].code) {
                if (options[i].name) return options[i].name;
                else break;
            }
        }
    }
    if (code > 0 && code < 256) buf[0] = code;
    else buf[0] = '?';
    return buf;
}

void zerostring(char *s) {
     while(*s) *s++ = 0;
}

int base91_encoding(unsigned char const *src, size_t srclength, void *target, size_t targsize) {
    static struct basE91 b91;
    size_t s;
    int total_size = 0;

    // zero the output buffer
    memset(target,'\0',targsize);

    // setup the base91 struct
    basE91_init(&b91);

    // encode most of it and keep the size
    s = basE91_encode(&b91, src, srclength, target);
    total_size += s;

    // pickup anything left and keep the size
    s = basE91_encode_end(&b91, (void*)(target+s));
    total_size += s;

    // return the total size
    return total_size;
}

int encode(char const *encoding, unsigned char const *src,
           size_t srclength, char *target, size_t targsize) {
    if (strcmp(encoding, "z85") == 0)
        return libscrypt_z85_encode(src, srclength, target, targsize);
    else if (strcmp(encoding, "base64") == 0)
        return libscrypt_b64_encode(src, srclength, target, targsize);
    else if (strcmp(encoding, "hex") == 0)
        return libscrypt_hex_encode(src, srclength, target, targsize);
    else if (strcmp(encoding, "dec") == 0)
        return libscrypt_b10_encode(src, srclength, target, targsize);
    else if (strcmp(encoding, "skey") == 0)
        return libscrypt_skey_encode(src, srclength, target, targsize);
    else if (strcmp(encoding, "b91") == 0)
        return base91_encoding(src, srclength, target, targsize);
    else
        return -1;
}

int main(const int argc, const char * const argv[]) {
    char * name                                 = NULL;
    char * password                             = NULL;
    char * site                                 = NULL;
    char registration_mode                      = 0;
    const char * filename                       = NULL;
    int  keylen                                 = SCRYPT_HASH_LEN;
    int  cache_cost                             = SCRYPT_CACHE_COST;
    int  cost                                   = SCRYPT_COST;
    int  scrypt_r                               = SCRYPT_r;
    int  scrypt_p                               = SCRYPT_p;
    char dry_run                                = 0;
    char * encoding                             = DEFAULT_ENCODING;
    char single_function_derivation             = 0;
    char verbose_lvl                            = 0;

    char cache_hash_in_file                     = 0;
    char b64buf[SCRYPT_HASH_LEN_MAX * 2]        = {0};
    char fpath[256]                             = {0};
    char error_msg[256]                         = {0};
    char verbose_msg[SCRYPT_HASH_LEN_MAX + 256] = {0};
    char mix_name_site[2032]                    = {0};
    const char * homedir                        = NULL;
    const char *encodings[]                     = {
        "dec", "hex", "base64", "z85", "skey", "b91", NULL };
    FILE  *fp                                   = NULL;
    int   i, readbytes, argi                    = 0;
    uint64_t cache_scrypt_n                     = 0;
    uint64_t scrypt_n                           = 0;

    uint8_t cache_hashbuf[SCRYPT_HASH_LEN_MAX]  = {0};
    uint8_t hashbuf[SCRYPT_HASH_LEN_MAX]        = {0};

    struct rlimit rlim;
    struct Arg_parser parser;
    const struct ap_Option options[] = {
      { 'n', "name",                ap_yes },
      { 'p', "password",            ap_yes },
      { 's', "site",                ap_yes },
      { 'r', "registration-mode",   ap_no  },
      { 'f', "file",                ap_yes },
      { 'l', "key-length",          ap_yes },
      { 'C', "cache-cost",          ap_yes },
      { 'c', "cost",                ap_yes },
      { 200, "scrypt-r",            ap_yes },
      { 201, "scrypt-p",            ap_yes },
      { 'N', "dry-run",             ap_no  },
      { 'e', "encoding",            ap_yes },
      { '1', "single",              ap_no  },
      { 'v', "verbose",             ap_no  },
      { 'V', "version",             ap_no  },
      { 'h', "help",                ap_no  } };

    //prevent leaving memory dumps
    //passwords won't be recoverable on memory crashes
    getrlimit(RLIMIT_CORE, &rlim);
    rlim.rlim_max = rlim.rlim_cur = 0;
    if (setrlimit(RLIMIT_CORE, &rlim)) exit(EXIT_FAILURE);
    //---------------------------------------------------

    //process options
    if (!ap_init(&parser, argc, argv, options, 0))
        die("Not enough memory.", 0, 0);
    //unrecognized option
    if (ap_error(&parser)) die(ap_error(&parser), 0, 1);

    for (argi = 0; argi < ap_arguments (&parser); ++argi) {
        const int code = ap_code(&parser, argi);
        const char * const arg = ap_argument(&parser, argi);
        if (code) {
            switch (code) {
                case 'n': if (arg[0]) { name     = (char *) arg; } break;
                case 'p': if (arg[0]) { password = (char *) arg; } break;
                case 's': if (arg[0]) { site     = (char *) arg; } break;
                case 'r': registration_mode = 1; break;
                case 'f': if (arg[0]) { filename = arg; } break;
                case 'l': if (arg[0]) {
                    if (strtol(arg, NULL, 10) <= 0) {
                        snprintf(error_msg, sizeof error_msg, \
                            "option '-l' requires a numerical argument, '%s'", arg);
                        die(error_msg, 0, 1);
                    } else {
                        keylen = (int) strtol(arg, NULL, 10);
                        if (keylen < SCRYPT_HASH_LEN_MIN || \
                            keylen > SCRYPT_HASH_LEN_MAX) {
                            snprintf(error_msg, sizeof error_msg, \
                                "option '-l' numerical value must be between %d-%d, '%d'", \
                                SCRYPT_HASH_LEN_MIN, SCRYPT_HASH_LEN_MAX, keylen);
                            die(error_msg, 0, 1);
                        }
                    } } break;

                case 'C': if (arg[0]) {
                    if (strtol(arg, NULL, 10) <= 0) {
                        snprintf(error_msg, sizeof error_msg, \
                            "option '-C' requires a numerical argument, '%s'", arg);
                        die(error_msg, 0, 1);
                    } else {
                        cache_cost = (int) strtol(arg, NULL, 10);
                        if (cache_cost > SCRYPT_SAFE_N) {
                            snprintf(error_msg, sizeof error_msg, \
                                "option '-C' numerical value must be between 1-%d, '%d'", \
                                SCRYPT_SAFE_N, cache_cost);
                            die(error_msg, 0, 1);
                        }
                    } } break;

                case 'c': if (arg[0]) {
                    if (strtol(arg, NULL, 10) <= 0) {
                        snprintf(error_msg, sizeof error_msg, \
                            "option '-c' requires a numerical argument, '%s'", arg);
                        die(error_msg, 0, 1);
                    } else {
                        cost = (int) strtol(arg, NULL, 10);
                        if (cost > SCRYPT_SAFE_N) {
                            snprintf(error_msg, sizeof error_msg, \
                                "option '-c' numerical value must be between 1-%d, '%d'", \
                                SCRYPT_SAFE_N, cost);
                            die(error_msg, 0, 1);
                        }
                    } } break;

                case 200: if (arg[0]) {
                    if (strtol(arg, NULL, 10) <= 0) {
                        snprintf(error_msg, sizeof error_msg, \
                            "option '--scrypt-r' requires a numerical argument, '%s'", arg);
                        die(error_msg, 0, 1);
                    } else {
                        scrypt_r = (int) strtol(arg, NULL, 10);
                        if (scrypt_r > SCRYPT_SAFE_r) {
                            snprintf(error_msg, sizeof error_msg, \
                                "option '--scrypt-r' numerical value must be between 1-%d, '%d'", \
                                SCRYPT_SAFE_r, scrypt_r);
                            die(error_msg, 0, 1);
                        }
                    } } break;

                case 201: if (arg[0]) {
                    if (strtol(arg, NULL, 10) <= 0) {
                        snprintf(error_msg, sizeof error_msg, \
                            "option '--scrypt-p' requires a numerical argument, '%s'", arg);
                        die(error_msg, 0, 1);
                    } else {
                        scrypt_p = (int) strtol(arg, NULL, 10);
                        if (scrypt_p > SCRYPT_SAFE_p) {
                            snprintf(error_msg, sizeof error_msg, \
                                "option '--scrypt-p' numerical value must be between 1-%d, '%d'", \
                                SCRYPT_SAFE_p, scrypt_p);
                            die(error_msg, 0, 1);
                        }
                    } } break;

                case 'N': dry_run = 1; break;
                case 'e': if (arg[0]) {
                    int valid_encoding = 0;
                    encoding = (char *) arg;
                    char * orig_encoding_input = malloc(1+strlen(encoding));
                    if (orig_encoding_input)
                        for (i=0; encoding[i]; ++i) //strcpy
                            orig_encoding_input[i] = encoding[i];
                    else  { snprintf(error_msg, sizeof error_msg, \
                                "malloc() error: %s", strerror(errno));
                            die(error_msg, 0, 0); }
                    for (i=0; encoding[i]; ++i) encoding[i] = tolower(encoding[i]);
                    const char ** pencodings  = encodings;
                    while(*pencodings != NULL) {
                        if (strcmp(encoding,*pencodings) == 0) valid_encoding = 1;
                        pencodings++;
                    }
                    if(!valid_encoding) {
                        snprintf(error_msg, sizeof error_msg, \
                            "invalid text encoding '%s'", orig_encoding_input);
                        die(error_msg, 0, 1);
                    } free(orig_encoding_input); } break;
                case '1': single_function_derivation = 1; break;
                case 'v': verbose_lvl += 1; break;
                case 'V': version(); break;
                case 'h': usage(EXIT_SUCCESS); break;
                default : die("uncaught option.", 0, 1);
            }
        } else { if (arg[0]) site = (char *) arg; }
    }

    //initialize missing options
    if (name == NULL)
        if (tarsnap_readinput(&name, "Name", NULL, 1))
            die("tarsnap_readinput() error.", 0, 0);
    if (site == NULL)
        if (tarsnap_readinput(&site, "Site", NULL, 1))
            die("tarsnap_readinput() error.", 0, 0);
    if (password == NULL) {
        if (registration_mode) {
            if (tarsnap_readpass(&password, "Master password", "repeat again", 1))
                die("tarsnap_readpass() error.", 0, 0);
        } else {
            if (tarsnap_readpass(&password, "Master password", NULL, 1))
                die("tarsnap_readpass() error.", 0, 0);
        }
    }

    if (filename == NULL) {
        if ((homedir = getenv("HOME")) != NULL) {
            snprintf(fpath, sizeof fpath, "%s/%s", homedir, ".genpass-cache");
            filename = fpath;
        } else {
            fprintf(stderr, "Warning: unable to determinate HOME directory, ");
            fprintf(stderr, "falling to --dry-mode ...\n");
            dry_run = 1;
        }
    }

    //Scheme: https://www.cs.utexas.edu/~bwaters/publications/papers/www2005.pdf
    //
    //This scheme uses two levels of hash computations (although with the -1
    //parameter it can use only one). The first level is executed once when a
    //user begins to use a new machine for the first time. This computation is
    //parameterized to take a relatively long time (around 60 seconds on this
    //implementation) and its result are cached for future password
    //calculations by the same user. The next level is used to compute
    //site-specific passwords. It takes as input the calculation produced from
    //the first level as well as the name of the site or account for which the
    //user is interested in generating a password, the computation time is
    //parameterized to be fast (around .1 seconds in our implementation).
    //
    //Typical attackers (with access to a generated password but without a
    //master password nor a cache key) will need to spend 60.1 seconds on
    //average per try and with little room for parallelization while legitimate
    //users will require 0.1s. This way the scheme strives for the best balance
    //between security and usability.
    //
    //The scheme has been updated to use a key derivation function specifically
    //designed to be computationally intensive on CPU, RAM and custom hardware
    //attacks, scrypt[0]. The original paper uses a sha1 iteration logarithm
    //which can be parallelized and is fast on modern hardware[1](2010), fast
    //is bad on key derived functions.
    //
    //[0] http://www.tarsnap.com/scrypt/scrypt.pdf
    //[1] https://software.intel.com/en-us/articles/improving-the-performance-of-the-secure-hash-algorithm-1

    cache_scrypt_n = _pow(2,cache_cost);
    scrypt_n       = _pow(2,cost);

    if (!single_function_derivation && !dry_run) {
        fp = fopen(filename, "rb");
        snprintf(verbose_msg, sizeof(verbose_msg), "Trying to open %s", filename);
        verbose(verbose_msg, verbose_lvl);
        if (fp!=NULL) {
            verbose("File open, attempting to read cache key", verbose_lvl);

            //probably it'll be a better to use a modular crypt format (mcf)
            //when it's defined for scrypt
            //http://mail.tarsnap.com/scrypt/msg00218.html

            //basic attempt to find a valid key by size
            fseek(fp, 0, SEEK_END);
            if (ftell(fp) == (keylen*(cache_cost+scrypt_r+scrypt_p))) {
                rewind(fp);
                readbytes = 0; for (i = 0; i < (cache_cost+scrypt_r+scrypt_p); i++)
                    readbytes = fread(cache_hashbuf,1,keylen,fp);
                if (readbytes != keylen) {
                    fprintf(stderr, "Warning: error while reading %s, ", filename);
                    fprintf(stderr, "falling to --dry-mode ...\n");
                    dry_run = 1;
                } else {
                    if (libscrypt_b64_encode_compliant(cache_hashbuf, keylen, \
                        b64buf, sizeof(b64buf)) == -1) {
                        snprintf(b64buf, sizeof(b64buf), "0");
                    }
                    verbose("Loaded valid cache key value", verbose_lvl);
                    cache_hash_in_file = 1;
                }
            } else verbose("Invalid cache key value", verbose_lvl);
            fclose(fp);
        } else if (errno == ENOENT) verbose("No such file", verbose_lvl);
        else verbose("Permission denied", verbose_lvl);
    }

    if (!single_function_derivation) {
        if (libscrypt_b64_decode_compliant(b64buf, cache_hashbuf, keylen) <= 0) {
            cache_hash_in_file = 0;
            verbose("Generating new cache key ...", verbose_lvl);
            if (libscrypt_scrypt((uint8_t *) password, (size_t) strlen(password), \
                    (uint8_t *) name, (size_t) strlen(name), \
                    cache_scrypt_n, scrypt_r, scrypt_p,      \
                    cache_hashbuf, keylen))   {
                snprintf(error_msg, sizeof error_msg,        \
                    "libscrypt_scrypt() failed: %s", strerror(errno));
                die(error_msg, 0, 0);
            }
        }
    }

    if (!single_function_derivation && !dry_run && !cache_hash_in_file) {
        snprintf(verbose_msg, sizeof(verbose_msg), \
            "Attempting to save cache key to %s", filename);
        verbose(verbose_msg, verbose_lvl);
        fp = fopen(filename, "wb");
        if (fp!=NULL) {
            for (i = 0; i < (cache_cost+scrypt_r+scrypt_p); i++) {
                if (fwrite(&cache_hashbuf, keylen, 1, fp) != 1) {
                    fprintf(stderr, "Warning: error while writing %s, ", filename);
                    fprintf(stderr, "falling to --dry-mode ...\n");
                    dry_run = 1;
                    break;
                }
            }
            fclose(fp);
            cache_hash_in_file = 1;
        } else verbose("Permission denied", verbose_lvl);
    }

    if (single_function_derivation) {
        verbose("Generating single derived key ...", verbose_lvl);
        snprintf(mix_name_site, sizeof(mix_name_site), "%s%s", name, site);
        name = mix_name_site;
    } else {
        zerostring(name);
        if (libscrypt_b64_encode_compliant(cache_hashbuf, keylen, b64buf, sizeof(b64buf)) == -1) {
            snprintf(error_msg, sizeof error_msg, \
                "libscrypt_b64_encode_compliant() failed: %s", strerror(errno));
            die(error_msg, 0, 0);
        }
        snprintf(verbose_msg, sizeof(verbose_msg), "Cache key: %s", b64buf);
        verbose(verbose_msg, verbose_lvl);
        verbose("Generating double derived key ...", verbose_lvl);
        snprintf(mix_name_site, sizeof(mix_name_site), "%s%s", cache_hashbuf, site);
        name = mix_name_site;
    }

    if (libscrypt_scrypt((uint8_t *) password, (size_t) strlen(password), \
            (uint8_t *) name, (size_t) strlen(name), \
            scrypt_n, scrypt_r, scrypt_p, hashbuf, keylen)) {
        snprintf(error_msg, sizeof error_msg, \
            "libscrypt_scrypt() failed: %s", strerror(errno));
        die(error_msg, 0, 0);
    }

    zerostring(name);
    zerostring(site);
    zerostring(password);

    if (encode(encoding, hashbuf, keylen, b64buf, sizeof(b64buf)) == -1) {
        snprintf(error_msg, sizeof error_msg, \
            "encode(%s) failed: %s", encoding, strerror(errno));
        die(error_msg, 0, 0);
    }

    fprintf(stdout, "%s\n", b64buf);
    return 0;
}
