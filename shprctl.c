#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#define SIZE 2 // size of one 
/* Numbers of significant redisters*/
#define STAT_REG 0 //port status, enabled|disabled
#define RATE_REG 1 //speed rate
#define PRIV_REG 4 //traffic priviledge

/* Speed coefficient, bps per token */
#define coeff 250000

/* 1 Gb is the maximum acceptable speed rate */
#define MAX_SPEED 1000000000

const char * cr_base ="/sys/class/fpga/fpga0/device/features/RX_RATE_LIMIT/cr_base";
const char * cr_cnt ="/sys/class/fpga/fpga0/device/features/RX_RATE_LIMIT/cr_cnt";
const char * sr_base ="/sys/class/fpga/fpga0/device/features/RX_RATE_LIMIT/sr_base";
const char * sr_cnt ="/sys/class/fpga/fpga0/device/features/RX_RATE_LIMIT/sr_cnt";

struct globalArgs_t {
	int port;
    int sector;
    int status;
    int rate;
    int priv;
    int flag;				

} globalArgs;

static const char *optString = "P:r:p:edshv?";

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

int regs(FILE *f, const char *path)
{
	char y[16];
	int x = 0, cl; 

	if((f=fopen(path, "rb"))==NULL) 
	{
		printf("Cannot open file cr_cnt.\n");
		return 1;
	}

	if(fgets(y, sizeof(y), f))
	{
		x = atoi(y);
	}

	cl = fclose(f);
	return x;
}

int is_enabled(int addr)
{
    FILE *fp;
    unsigned short val = 0, f;
	if((fp=fopen("/dev/etn", "rb"))==NULL) 
	{
		printf("Cannot open file.");
		return 1;
	}
	
    if(fseek(fp, addr*SIZE, SEEK_SET) )
	{
		printf("seek error\n");
		return 1;
	}
	if(fread(&val, 2, 1, fp) != 1)
	{
		printf("read error\n");
		return 1;
	}
    
	int c = fclose(fp);
	return val%2;
}

int reg_write(int addr, unsigned short val) 
{
	FILE *fp;

	if((fp=fopen("/dev/etn", "wb"))==NULL) 
	{
		printf("Cannot open file.");
		return 1;
	}

    if(fseek(fp, addr*SIZE, SEEK_SET)) 
    {
        printf("seek error\n");
        return 1;
    }

	if(fwrite(&val, 2, 1, fp) != 1)
	{
		printf("Error write.\n");
		return 1;
	}

	int c = fclose(fp);
	return 0;
}

int enable(int addr)
{
    reg_write(addr, 1);

    return 0;
}

int disable(int addr)
{
    reg_write(addr, 0);

    return 0;
}


int reg_read(int addr)
{
	FILE *fp;
    unsigned short val = 0;
	if((fp=fopen("/dev/etn", "rb"))==NULL) 
	{
		printf("Cannot open file.");
		return 1;
	}
	
    if(fseek(fp, addr*SIZE, SEEK_SET) )
	{
		printf("seek error\n");
		return 1;
	}
	if(fread(&val, 2, 1, fp) != 1)
	{
		printf("read error\n");
		return 1;
	}

	int c = fclose(fp);
	return val;
}

int get_rate(int addr)
{
    int value;
    value = reg_read(addr+RATE_REG)*coeff;

    return value;
}

char * get_priv(int addr)
{
    char * value;
    if(reg_read(addr + PRIV_REG)==0)
        value = "both";
    else if(reg_read(addr + PRIV_REG) == 1)
        value  = "user";
    else if(reg_read(addr + PRIV_REG)==2)
        value = "mpt";
    else if(reg_read(addr + PRIV_REG)==3)
        value = "none";
    
    return value;
}

int set_rate(int addr, int rate)
{
    reg_write(addr + RATE_REG, rate);

    return 0;
}

int set_priv(int addr, int priv)
{
    reg_write(addr + PRIV_REG, priv);

    return 0;
}

int define_priv(const char *priv)
{
    int x;
    
    if(strncmp(priv, "both", 16)==0)
        x = 0;
    else if (strncmp(priv, "user", 16)==0)
        x = 1;
    else if (strncmp(priv, "mpt", 16)==0)
        x = 2;
    else if (strncmp(priv, "none", 16)==0)
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
    printf("Usage: %s --port = {0|1} [OPTIONS]\n", progname);
    printf("Arguments:\n");
    printf("\t-p\t--set-priv = {both|user|mpt|none}\n");
    printf("\t-r\t--set-rate = {Speed rate, bps}\n");
    printf("\t-e\t--enable\n");
    printf("\t-d\t--disable\n");
    printf("\t-s\t--status\n");

    return 0;
}

int display_status()
{
    char * str;
    printf("Port %d\n", globalArgs.port);
    if(is_enabled(globalArgs.sector))
        printf("Shaper status:\t\tenabled\n");
    else
        printf("Shaper status:\t\tdisabled\n");
        
    printf("Speed rate, bps:\t%d\n", get_rate(globalArgs.sector));

    printf("Traffic privilege:\t%s\n", get_priv(globalArgs.sector));
    
    return 0;
}

int get_options(int argc, char *argv[])
{
    int opt = 0, longIndex = 0;
    double s;

    globalArgs.port = -1;
    globalArgs.sector = 0;
    globalArgs.status = 0;
    globalArgs.rate = 0;
    globalArgs.priv = -1;
    globalArgs.flag = -1;

    
    while( opt != -1 ) {
		opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
        switch( opt ) {
		    case 'P': //port = {0..1};
                globalArgs.port = atoi(optarg);
                if ((globalArgs.port != 0) && (globalArgs.port != 1))
                    printf("Wrong port number. Port can have values only 0 or 1.\n");
				break;

			case 's': //Port and it's status(on/off), rate(bps) and priviledges.
                globalArgs.status = 1;                
                break;
				
			case 'r': // set-rate = {250000 .. 10^9};
                s = atoi(optarg);
                    if (s>=MAX_SPEED)
                    {
                        printf("The rate should be less than %d.\n", MAX_SPEED);
                        return 1;
                    }
                    else if(atoi(optarg)<coeff)
                    {
                        printf("The minimal rate is %d.\n", coeff);
                        return 1;
                    } 
                    else
                    globalArgs.rate = s/coeff;            
				break;
				
			case 'p': // set-priv = {both/user/mpt/none}
                globalArgs.priv = define_priv(optarg); 
				break;

			case 'e': //enable
                globalArgs.flag = 1;
				break;
                				
			case 'd': //disable
                globalArgs.flag = 0;
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
                display_usage(argv[0]);
                exit(1);
                break;
			case 0:
                printf("Case 0. \n");
				display_usage(argv[0]);
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
        exit(0);
    }
    return 0;
}

int main(int argc, char *argv[])
{
	FILE *cr_b, *cr_c, *sr_b, *sr_c;
	int sector, numread, c;
	int crb, crc, srb, src, address, port, index;
	unsigned short  str = 0;
	
    c = get_options(argc, argv);

	crb = regs(cr_b, cr_base);
	crc = regs(cr_c, cr_cnt);
	srb = regs(sr_b, sr_base);
	src = regs(sr_c, sr_cnt);
	
    unsigned int speed = 0;       
    
    if(globalArgs.port==0)
        globalArgs.sector = crb + STAT_REG;
    else 
        if(globalArgs.port==1)
            globalArgs.sector = crb + crc + STAT_REG;

    if(argc<=2)
    {
        display_usage(argv[0]);
        exit(0);
    }

    if(globalArgs.flag==1)
        enable(globalArgs.sector);
    else if (globalArgs.flag==0)
        disable(globalArgs.sector);

    if(globalArgs.rate > 0)
        if(is_enabled(globalArgs.sector))
        {
            disable(globalArgs.sector);
            set_rate(globalArgs.sector, globalArgs.rate);
            enable(globalArgs.sector);
        }
        else 
            set_rate(globalArgs.sector, globalArgs.rate);
            
    if(globalArgs.priv!=-1)
        if(is_enabled(globalArgs.sector))
        {
            disable(globalArgs.sector);
            set_priv(globalArgs.sector, globalArgs.priv);
            enable(globalArgs.sector);
        }
        else
            set_priv(globalArgs.sector, globalArgs.priv);
    
    if(globalArgs.status)
        display_status();

	return 0;
}

