#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <getopt.h>
#define SIZE 2

const char * cr_base ="/sys/class/fpga/fpga0/device/features/RX_RATE_LIMIT/cr_base";
const char * cr_cnt ="/sys/class/fpga/fpga0/device/features/RX_RATE_LIMIT/cr_cnt";
const char * sr_base ="/sys/class/fpga/fpga0/device/features/RX_RATE_LIMIT/sr_base";
const char * sr_cnt ="/sys/class/fpga/fpga0/device/features/RX_RATE_LIMIT/sr_cnt";

// char buf[SIZE];
// struct globalArgs_t {
// 	int port;				

// } globalArgs;

// static const char *optString = "P:S:p:edsh?";

// static const struct option longOpts[] = {
// 	{ "port", required_argument, NULL, 'P' },
// 	{ "set-rate", required_argument, NULL, 'S' },
// 	{ "set-priv", required_argument, NULL, 'p' },
// 	{ "enable", no_argument, NULL, 'e' },
// 	{ "disable", no_argument, NULL, 'd' },
// 	{ "status", no_argument, NULL, 's' },
// 	{ "help", no_argument, NULL, 'h' },
// 	{ NULL, no_argument, NULL, 0 }
// };

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
		printf("%s: %d\n", path, x);
	}
	cl = fclose(f);
	return x;
}

int reg_write(int addr, unsigned short val) 
{
	FILE *fp;

	if((fp=fopen("/dev/etn", "wb"))==NULL) 
	{
		printf("Cannot open file.");
		return 1;
	}

	if(fseek(fp, addr, SEEK_SET)) 
	{
		printf("seek error\n");
		return 1;
	}

	printf("We are at %d\n", ftell(fp));


	if(fwrite(&val, 2, 1, fp) != 1)
	{
		printf("\n");
		return 1;
	}

	return 0;
}

int reg_read(int addr, __uint16_t *val)
{

	FILE *fp;

	if((fp=fopen("/dev/etn", "rb"))==NULL) 
	{
		printf("Cannot open file.");
		return 1;
	}

	if(fseek(fp, addr, SEEK_SET)) 
	{
		printf("seek error\n");
		return 1;
	}

	printf("We are at %d\n", ftell(fp));

	
	if(fread(&val, 2, 1, fp) != 1)
	{
		printf("\n");
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fp, *cr_b, *cr_c, *sr_b, *sr_c;
	int sector, numread, c;
	int crb, crc, srb, src;
	unsigned short val = 0, str;
	
	crb = regs(cr_b, cr_base);
	crc = regs(cr_c, cr_cnt);
	srb = regs(sr_b, sr_base);
	src = regs(sr_c, sr_cnt);

	if((fp=fopen("/dev/etn", "w+"))==NULL) 
	{
		printf("Cannot open file.");
		return 1;
	} 

	// if(argc<2) 
	// {
	// 	printf("Usage: dump filename\n");
	// 	return 1;
	// }


	// do
	// int port =  atoi(argv[1]);
	sector = crb + atoi(argv[1]);
	// if(port==0)
	// 	sector = crb;
	// else if(port==1)
	// 	sector = crb+crc;
	// else printf("error\n");

	str = atoi(argv[2]);

	// if((argc>=3)&(sector>=0))
	// {
		
		// printf("Enter register number: ");
		// scanf("%d", &sector);
		if(sector >= 0) 
		{
			if(fseek(fp, sector*SIZE, SEEK_SET)) 
			{
				printf("seek error\n");
			}			
			numread=fread(&val, 2, 1, fp);
			printf("Numread %d\n", numread);
			// printf("0x%x: 0x%x and move to %d\n", sector * SIZE, val, ftell(fp));


			if(fseek(fp, sector*SIZE, SEEK_SET)) 
			{
				printf("seek error\n");
			}

			numread=fwrite(&str, 2, 1, fp);
			printf("Numread %d\n", numread);
			printf("0x%x: 0x%x at %d\n", sector * SIZE, val, ftell(fp));

		}
	// } 

	c = fclose(fp);
	return 0;
}

