#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>
/* register size in bytes*/
#define SIZE 2
/* Numbers of significant redisters*/
/* Port status, enabled|disabled*/
#define STAT_REG 0
/* Speed rate*/
#define RATE_REG 1
/* Traffic priviledge*/
#define PRIV_REG 4

/* Kinds of privileges.*/
#define BOTH "both"
#define USER "user"
#define MPT "mpt"
#define NONE "none"

/* Speed coefficient, bps per token */
#define COEFF 250000

/* Mask for enable/disable shaper.*/
#define MASK_EN 0x1
#define MASK_DIS 0xfffe
/* 1 Gb is the maximum acceptable speed rate */
#define MAX_SPEED 1000000000
/* Path to files cr_base, cr_cnt, sr_base, sr_cnt.*/
#define PATH_TO_REGS "/sys/class/fpga/fpga0/device/features/RX_RATE_LIMIT/"

const char * cr_base = PATH_TO_REGS"cr_base";
const char * cr_cnt = PATH_TO_REGS"cr_cnt";
const char * sr_base = PATH_TO_REGS"sr_base";
const char * sr_cnt = PATH_TO_REGS"sr_cnt";
const char * etn = "/dev/etn";

struct global_args_t {
    int port;
    int sector;
    int status;
    int rate;
    int priv;
    int flag;
} global_args;

static const char * optString = "P:r:p:edshv?";

static const struct option longOpts[] = {
    { "port", required_argument, NULL, 'P' },
    { "set-rate", required_argument, NULL, 'r' },
    { "set-priv", required_argument, NULL, 'p' },
    { "enable", no_argument, NULL, 'e' },
    { "disable", no_argument, NULL, 'd' },
    { "status", no_argument, NULL, 's' },
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'v' },
    { NULL, no_argument, NULL, 0 }
};

/* Function read_offsets reads from files that contains positions of shaper's registers.*/
int read_offsets(FILE * f, const char * path)
{
    char y[16];
    int x = 0, cl;

    if ((f = fopen(path, "rb")) == NULL)
    {
        printf("Cannot open file %s.\n", path);
        return -1;
    }

    if (fgets(y, sizeof(y), f))
    {
        x = atoi(y);
    }

    cl = fclose(f);
    return x;
}

int reg_write(int addr, uint16_t val) 
{
    FILE * fp;

    if ((fp = fopen(etn, "wb")) == NULL) 
    {
        printf("Cannot open file.");
        return -1;
    }

    if (fseek(fp, addr*SIZE, SEEK_SET))
    {
        printf("seek error\n");
        return -1;
    }

    if (fwrite(&val, 2, 1, fp) != 1)
    {
        printf("Error write.\n");
        return -1;
    }

    int cl = fclose(fp);
    return 0;
}

int reg_read(int addr, uint16_t * val)
{
    FILE * fp;

    if ((fp = fopen(etn, "rb")) == NULL)
    {
        printf("Cannot open file.");
        return -1;
    }

    if (fseek(fp, addr*SIZE, SEEK_SET) )
    {
        printf("seek error\n");
        return -1;
    }
    if (fread(val, 2, 1, fp) != 1)
    {
        printf("read error\n");
        return -1;
    }

    int cl = fclose(fp);
    return 0;
}

int enable(int addr)
{
    int val;
    uint16_t t = 0;
    
    if (val = reg_read(addr, &t) == -1)
    {
        printf("read error\n");
        return -1;
    }
    else val = t | MASK_EN;

    if (reg_write(addr, val) == -1)
    {
        printf("write error\n");
        return -1;
    }

    return 0;
}

int disable(int addr)
{
    int val;
    uint16_t t = 0;

    if (val = reg_read(addr, &t) == -1)
    {
        printf("read error\n");
        return -1;
    }
    else val = t & MASK_DIS;

    if (reg_write(addr, val) == -1)
    {
        printf("write error\n");
        return -1;
    }

    return 0;
}

int is_enabled(int addr)
{
    uint16_t t = 0;  

    if (reg_read(addr, &t) == -1)
    {
        printf("read error\n");
        return -1;
    }

    return (t & MASK_EN);
}

int get_rate(int addr, uint16_t * val)
{
    if (reg_read(addr + RATE_REG, val) == -1)
    {
        printf("read error\n");
        return -1;
    }

    return 0;
}

char * get_priv(int addr)
{
    char * value;
    uint16_t t;
    if (reg_read(addr + PRIV_REG, &t) == -1)
    {
        printf("read error\n");
        return NULL;
    }

    switch (t)
    {
    case 0:
        value = BOTH;
        break;
    case 1:
        value  = USER;
        break;
    case 2:
        value = MPT;
        break;
    case 3:
        value = NONE;
        break;
    default:
        value = NULL;
        break;
    }

    return value;
}

int set_rate(int addr, int rate)
{
    if (reg_write(addr + RATE_REG, rate) == -1)
    {
        printf("write error\n");
        return -1;
    }

    return 0;
}

int set_priv(int addr, int priv)
{
    if (reg_write(addr + PRIV_REG, priv) == -1)
    {
        printf("write error\n");
        return -1;
    }

    return 0;
}

int define_priv(const char * priv)
{
    int x;

    if (strncmp(priv, BOTH, sizeof(BOTH)) == 0)
        x = 0;
    else if (strncmp(priv, USER, sizeof(USER)) == 0)
        x = 1;
    else if (strncmp(priv, MPT , sizeof(MPT)) == 0)
        x = 2;
    else if (strncmp(priv, NONE, sizeof(NONE)) == 0)
        x = 3;
    else
    {
        x = -1;
        printf("Wrong argument 'set-priv'.\n");
    }

    return x;
}

int display_version()
{
    printf("Shaper control\ncontrol hardware traffic shaper\nVersion 2.0\n");
}

int display_usage(char *progname)
{
    printf("Shaper control\n");
    printf("%s\t-- control hardware traffic shaper\n\n", progname);
    printf("Usage: %s --port={0|1} [OPTIONS]\n", progname);
    printf("Arguments:\n");
    printf("\t-p\t--set-priv={both|user|mpt|none}\n");
    printf("\t-r\t--set-rate={Speed rate, bps}\n");
    printf("\t-e\t--enable\n");
    printf("\t-d\t--disable\n");
    printf("\t-s\t--status\n");

    return 0;
}

int display_status()
{
    uint16_t t;

    if ((global_args.port != 0) && (global_args.port != 1))
    {
        printf("Wrong port number.\n");
        return -1;
    }
    else 
        printf("Port %d\n", global_args.port);

    if (is_enabled(global_args.sector) == 1)
        printf("Shaper status:\t\tenabled\n");
    else if (is_enabled(global_args.sector) == 0)
        printf("Shaper status:\t\tdisabled\n");
    else
    {
        fprintf( stderr, "check is_enabled failed\n");
        return -1;
    }

    if (get_rate(global_args.sector, &t) == -1)
    {
        fprintf( stderr, "check rate failed\n");
        return -1;
    }
    else    
        printf("Speed rate, bps:\t%d\n", t*COEFF);

    if (get_priv(global_args.sector) == NULL)
    {
        fprintf( stderr, "check priv failed\n");
        return -1;
    }
    else
        printf("Traffic privilege:\t%s\n", get_priv(global_args.sector));

    return 0;
}

int get_options(int argc, char *argv[])
{
    int opt = 0, longIndex = 0;
    double s;

    global_args.port = -1;
    global_args.sector = 0;
    global_args.status = 0;
    global_args.rate = 0;
    global_args.priv = -1;
    global_args.flag = -1;

    while( opt != -1 ) {
        opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
        switch( opt ) {
            case 'P': //port = {0..1};
                if (strncmp(optarg, "0", 2) == 0)
                    global_args.port = 0;
                else if (strncmp(optarg, "1", 2) == 0)
                    global_args.port = 1;
                else
                {
                    printf("Wrong port number. Port can have values only 0 or 1.\n");
                    exit(-1);
                }
                break;
            case 's': //Port and it's status(on/off), rate(bps) and priviledges.
                if ((global_args.port == 0) || (global_args.port == 1))
                    global_args.status = 1;
                else
                {
                    printf("Port number is incorrect. Port can have values only 0 or 1.\n");
                    exit(-1);
                }                
                break;
            case 'r': // set-rate = {250000 .. 10^9};
                s = atoi(optarg);
                    if (s >= MAX_SPEED)
                    {
                        printf("The rate should be less than %d.\n", MAX_SPEED);
                        return 1;
                    }
                    else if (atoi(optarg) < COEFF)
                    {
                        printf("The minimal rate is %d.\n", COEFF);
                        return 1;
                    }
                    else
                    global_args.rate = s/COEFF;
                break;
            case 'p': // set-priv = {both/user/mpt/none}
                global_args.priv = define_priv(optarg);
                break;
            case 'e': //enable
                global_args.flag = 1;
                break;
            case 'd': //disable
                global_args.flag = 0;
                break;
            case 'v':
                display_version();
                exit(0);
                break;
            case 'h':
                display_usage(argv[0]);
                exit(0);
                break;
            case '?':
                exit(-1);
                break;
            case 0:
                return 1;
                break;
            default:
                break;
        }
    }

    if (optind < argc) {
        printf("Wrong arguments: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
        display_usage(argv[0]);
        exit(-1);
    }
    return 0;
}

int main(int argc, char * argv[])
{
    FILE * cr_b, * cr_c, * sr_b, * sr_c;
    int sector, numread, c;
    int crb, crc, srb, src, address, port, index;

    c = get_options(argc, argv);

    crb = read_offsets(cr_b, cr_base);
    crc = read_offsets(cr_c, cr_cnt);
    srb = read_offsets(sr_b, sr_base);
    src = read_offsets(sr_c, sr_cnt);

    unsigned int speed = 0;

    switch (global_args.port)
    {
    case 0:
        global_args.sector = crb + STAT_REG;
        break;
    case 1:
        global_args.sector = crb + crc + STAT_REG;
        break;
    default:
        break;
    }

    if (argc <= 2)
    {
        printf("%s requires arguments.\n", argv[0]);
        display_usage(argv[0]);
        exit(-1);
    }

    switch (global_args.flag)
    {
    case 0:
        disable(global_args.sector);
        break;
    case 1:
        enable(global_args.sector);
        break;
    default:
        break;
    }

    if (global_args.rate > 0)
        if (is_enabled(global_args.sector) == 1)
        {
            disable(global_args.sector);
            set_rate(global_args.sector, global_args.rate);
            enable(global_args.sector);
        }
        else if (is_enabled(global_args.sector) == 0)
            set_rate(global_args.sector, global_args.rate);
        else
        {
            fprintf(stderr, "check is_enabled failed\n");
            return -1;
        }        

    if (global_args.priv != -1)
        if (is_enabled(global_args.sector) == 1)
        {
            disable(global_args.sector);
            set_priv(global_args.sector, global_args.priv);
            enable(global_args.sector);
        }
        else if (is_enabled(global_args.sector) == 0)
            set_priv(global_args.sector, global_args.priv);
        else
        {
            fprintf( stderr, "check is_enabled failed\n");
            return -1;
        }
            

    if (global_args.status == 1)
        display_status();

    return 0;
}

