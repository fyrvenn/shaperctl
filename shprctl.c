#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <getopt.h>
#define SIZE 2

const char * cr_base ="/sys/class/fpga/fpga0/device/features/RX_RATE_LIMIT/cr_base";
const char * cr_cnt ="/sys/class/fpga/fpga0/device/features/RX_RATE_LIMIT/cr_cnt";
const char * sr_base ="/sys/class/fpga/fpga0/device/features/RX_RATE_LIMIT/sr_base";
const char * sr_cnt ="/sys/class/fpga/fpga0/device/features/RX_RATE_LIMIT/sr_cnt";

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
    printf("0x%x: 0x%x and move to %d\n", addr, val, ftell(fp));

	int c = fclose(fp);
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
	int c = fclose(fp);
	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fp, *cr_b, *cr_c, *sr_b, *sr_c;
	int sector, numread, c;
	int crb, crc, srb, src, address;
	unsigned short val = 0;
    __uint16_t str;
	
	crb = regs(cr_b, cr_base);
	crc = regs(cr_c, cr_cnt);
	srb = regs(sr_b, sr_base);
	src = regs(sr_c, sr_cnt);


	str = atoi(argv[2]);
    address = crb + atoi(argv[1]);
    val = atoi(argv[2]);

    // numread = reg_write(address, val);
    // printf("Numwrite %d\n", numread);

        if(fseek(fp, sector*SIZE, SEEK_SET)) 
        {
            printf("seek error\n");
        }			
        numread=fread(&val, 2, 1, fp);
        printf("Numread %d\n", numread);
        printf("0x%x: 0x%x and move to %d\n", sector * SIZE, val, ftell(fp));


        // if(fseek(fp, sector*SIZE, SEEK_SET)) 
        // {
        //     printf("seek error\n");
        // }

        // numread=fread(&val, 2, 1, fp);
        // printf("Numread %d\n", numread);
        // printf("0x%x: 0x%x at %d\n", sector * SIZE, val, ftell(fp));

    // }

	return 0;
}

