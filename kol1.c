#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(){

	FILE *fp;
	char *str;
	char lval1,lval2,lval3,lval4;
	size_t num_of_bytes = 6;
	int led_vr = 0;

	FILE *fps;
	char *strs;
	char sval1,sval2,sval3,sval4;
	size_t num_of_bytess = 6;
	int sw_vr = 0;

	FILE *fpb;
	char *strb;
	char bval0;
	size_t num_of_bytesb = 6;

	int cnt = 0;
	int brojac=0;
	int rez = 0;
	int kk = 1;
	char rez_str [10];
	int bvali = 0;

	while(1){
	//Citanje vrednosti tastera
	fpb = fopen("/dev/button", "r");
	if(fpb==NULL) {
		puts("Problem pri otvaranju /dev/button");
		return -1;
	}
	strb = (char *)malloc (num_of_bytesb+1);
	getline(&strb, &num_of_bytesb, fpb);
	if(fclose(fpb)){
		puts("Problem pri zatvaranju /dev/button");
		return -1;
	}
	bval0 = strb[5] - 48;
	bvali = bval0 + 0;
	free(strb);

	if(bvali==1 && kk == 1){
		cnt++;
		kk = 0;
		usleep(10);

	}
	else if(bvali==0){
		kk = 1;
	}

	//Citanje vrednosti led
	if(brojac%20000==0){
	fp =fopen("/dev/led", "r");
	if(fp==NULL) {
		puts("Problem pri otvaranju /dev/led");
		return -1;
	}
	str = (char*)malloc(num_of_bytes+1);
	getline(&str, &num_of_bytes, fp);
	if(fclose(fp)){
		puts("Problem pri zatvaranju /dev/led");
		return -1;
	}
	lval1 = str[2] - 48;
	lval2 = str[3] - 48;
	lval3 = str[4] - 48;
	lval4 = str[5] - 48;
	free(str);
	led_vr = lval1 * 8 + lval2 * 4 + lval3 * 2 + lval4;
	printf("vrednost led:%d\n", led_vr);

	//Citanje vrednosti prekidaca
	fps = fopen("/dev/switch", "r");
	if(fps==NULL) {
		puts("Problem pri otvaranju /dev/switch");
	return -1;
	}
	strs = (char *)malloc (num_of_bytess+1);
	getline(&strs, &num_of_bytess, fps);
	if(fclose(fps)){
		puts("Problem pri zatvaranju /dev/switch");
	return -1;
	}
	sval1 = strs[2] - 48;
	sval2 = strs[3] - 48;
	sval3 = strs[4] - 48;
	sval4 = strs[5] - 48;
	free(strs);
	sw_vr = sval2 * 4  + sval3 * 2  + sval4;
	printf("vrednost prekidaca:%d\n", sw_vr);

	//racunanje rezultata

	if(sval1){
		if(cnt%2){
			rez = led_vr - sw_vr;
		}
		else{
			rez = led_vr + sw_vr;
		}
		if(rez>15){
			rez=15;
		}
		if(rez<0){
			rez=0;
		}
		printf("rez posle sabiranja: %d\n", rez);
	}

	// Upali diode
	//if(brojac%2000000==0){
	fp = fopen("/dev/led", "w");
	if(fp == NULL){
		printf("Problem pri otvaranju /dev/led\n");
		return -1;
	}

	sprintf(rez_str,"%d",rez);
	printf("%s\n",rez_str);

	fputs(rez_str, fp);
	if(fclose(fp)){
		printf("Problem pri zatvaranju /dev/led\n");
		return -1;
	}
	//sleep(2);
	}
	brojac++;
	}
	return 0;
}
