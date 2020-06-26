#include <stdio.h>
#include <stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>

typedef struct tagIMAGE_THUNK_DATA
{
	char* image_import_by_name;
} IMAGE_THUNK_DATA;


#pragma pack(push)
#pragma pack(2)
typedef struct tagIMAGE_IMPORT_BY_NAME{
	short hint;
	char name[20];
}IMAGE_IMPORT_BY_NAME;
#pragma pack(pop)

typedef struct tagIMAGE_IMPORT_DESCRIPTOR{
	IMAGE_THUNK_DATA* originalFirstthunk;
	int timestamp;
	int forwardchain;
	char* name;
	IMAGE_THUNK_DATA* firstthunk;
}IMAGE_IMPORT_DESCRIPTOR;

int modify_IID(){
#define NEW_IID_START 0x98000
#define INSERT_IAT_START 0x99000
#define INSERT_INT_START 0x99500
#define INSERT_IIBN_START 0x9A000

#define IID_START 0x14000
#define IID_ROFFS 0x10000
#define rsrc_v 0x15000
#define rsrc_r 0x10e00 
#define IID_SIZE 0x78
#define DLL_NAME 0x9A500

size_t old_iid_start = IID_ROFFS;
size_t new_iid_start = NEW_IID_START - rsrc_v + rsrc_r;
size_t new_iibn_start = INSERT_IIBN_START - rsrc_v + rsrc_r;
size_t new_int_start = INSERT_INT_START - rsrc_v + rsrc_r;
size_t new_iat_start = INSERT_IAT_START - rsrc_v + rsrc_r;
size_t dll_name = DLL_NAME - rsrc_v + rsrc_r;


size_t expand_size = 0x100000;
	FILE *sc,*pe;

	if((pe = fopen("./files/new_pika_IAT.exe","rb"))==NULL){
		printf("open failed\n");
		return 1;

	}
	int pe_size,sc_size;
	struct stat filestat;
	//
	fstat(fileno(pe),&filestat);
	pe_size = filestat.st_size;
	char *pe_data = (char*)malloc(expand_size);
	//
	memset(pe_data,0,expand_size);

	printf("pe_size=0x%x,sc_size=0x%x, c_size=%d\n",pe_size,sc_size,sizeof(char));
	fread(pe_data,1,pe_size,pe);


	memcpy(pe_data+new_iid_start,pe_data+ old_iid_start, IID_SIZE);

	IMAGE_IMPORT_DESCRIPTOR* new_iid_pos = (IMAGE_IMPORT_DESCRIPTOR*)(pe_data+new_iid_start+IID_SIZE-sizeof(IMAGE_IMPORT_DESCRIPTOR));

	IMAGE_IMPORT_BY_NAME * new_iibn_pos = (IMAGE_IMPORT_BY_NAME*)(pe_data+new_iibn_start);
	IMAGE_THUNK_DATA * new_int_pos = (IMAGE_THUNK_DATA *)(pe_data+new_int_start);
	IMAGE_THUNK_DATA * new_iat_pos = (IMAGE_THUNK_DATA *)(pe_data+new_iat_start);

	// set iibn
	new_iibn_pos[0] = (IMAGE_IMPORT_BY_NAME){1,"recv_wrap"};
	new_iibn_pos[1] = (IMAGE_IMPORT_BY_NAME){0,"create_socket"};
	new_iibn_pos[2] = (IMAGE_IMPORT_BY_NAME){2,"send_wrap"};
	new_iibn_pos[3] = (IMAGE_IMPORT_BY_NAME){3,"socket_exist"};
	new_iibn_pos[4] = (IMAGE_IMPORT_BY_NAME){4,"set_sever_client"};


	memcpy(pe_data+dll_name , "connect.dll\0",12);
	// set info of each new IID and ITD
	for(int i = 0 ;i<5;i++){


		new_int_pos[i].image_import_by_name = (char*)(INSERT_IIBN_START+sizeof(IMAGE_IMPORT_BY_NAME)*i);
		new_iat_pos[i].image_import_by_name = (char*)(INSERT_IIBN_START+sizeof(IMAGE_IMPORT_BY_NAME)*i);
		printf("addr of func-%s = 0x%x\n",new_iibn_pos[i].name,INSERT_IAT_START+sizeof(IMAGE_THUNK_DATA)*i);

	}
	new_iid_pos->originalFirstthunk = (IMAGE_THUNK_DATA*)(INSERT_INT_START);
	new_iid_pos->firstthunk = (IMAGE_THUNK_DATA*)(INSERT_IAT_START);
	new_iid_pos->name = DLL_NAME;

	// set IID entry
	*(int*)(pe_data+0xc0) = NEW_IID_START;


	FILE *out = fopen("./files/new_pika_IAT_mod.exe","wb");
	if(out==NULL){
		printf("output failed\n");
	}
	fwrite(pe_data,1,expand_size,out);

	fclose(out);
	return 0;	
}


int expand(){

#define expand_size 0x100000

	FILE *sc,*pe;

	if((pe = fopen("./files/pika_dbg.exe","rb"))==NULL){
		printf("open failed\n");
		return 1;

	}
	int pe_size,sc_size;
	struct stat filestat;
	//
	fstat(fileno(pe),&filestat);
	pe_size = filestat.st_size;
	char *pe_data = (char*)malloc(expand_size);
	//
	memset(pe_data,0,expand_size);

	printf("pe_size=0x%x,sc_size=0x%x, c_size=%d\n",pe_size,sc_size,sizeof(char));
	fread(pe_data,1,pe_size,pe);
	*(int*)(pe_data+0x1fc) = 0xE0000060;
	// new_func raw offset (start) = 0x85400
	//memcpy(pe_data+0x85400,sc_data,sc_size);
	*(int*)(pe_data+0x1e0) = 0x8A000;
	*(int*)(pe_data+0x1e8) = 0x8A000; // .rsrc vsize
	*(int*)(pe_data+0x90) = 0x100000;

	FILE *out = fopen("./files/new_pika.exe","wb");
	if(out==NULL){
		printf("output failed\n");
	}
	fwrite(pe_data,1,expand_size,out);

	fclose(out);
	return 0;	

}



int insert_func(){
#define FUNC_START 0x90000
#define NEW_IAT_START 0x98000
#define IAT_START 0x14000
#define rsrc_v 0x15000
#define rsrc_r 0x10e00 
// size=0x78
// auto_act = 0x402360
// call auto_act = 0x401fef end 0x401ff4
	int func_start = FUNC_START + rsrc_r - rsrc_v;
	int func_offset = FUNC_START - 0x401ff4+0x400000;
	FILE *sc,*pe;
	char ch;

	if((sc = fopen("./files/out","rb"))==NULL){
		printf("open failed\n");
		return 1;

	}

	if((pe = fopen("./files/new_pika.exe","rb"))==NULL){
		printf("open failed\n");
		return 1;

	}
	int pe_size,sc_size;
	struct stat filestat;
	//
	fstat(fileno(pe),&filestat);
	pe_size = filestat.st_size;
	char *pe_data = (char*)malloc(pe_size);
	//
	fstat(fileno(sc),&filestat);
	sc_size = filestat.st_size;
	char *sc_data = (char*)malloc(sc_size);

	printf("pe_size=0x%x,sc_size=0x%x, c_size=%d\n",pe_size,sc_size,sizeof(char));
	fread(pe_data,1,pe_size,pe);
	fread(sc_data,1,sc_size,sc);
//============^^prepare^^============================

	/*
	hook auto_act_proc to recv and send 
	*/
	// new_func raw offset (start) = 0x85400
	memcpy(pe_data+func_start,sc_data,sc_size); // copy func shell code to RVA:0x490000

	char *hook_auto_act_condition_flow = pe_data+0x200+0xfdd; // 401fdd - if(player->mode==1) (is computer?)
	memcpy(hook_auto_act_condition_flow,"\x83\xbe\xa4\x0\x0\x0\x0\x74",8); // if(player->mode!=0) call 0x490000 (set_server_or_client)

	int *hook_auto_act = (int*)(pe_data+0x200+0xfF0); //401ff0 - call auto_act
	*hook_auto_act = func_offset + 0x70;                     //=>call 0x490070 (send_or_recv)


	char *hook_2p_setting_constraint = pe_data+0x200+0xF1A; // 401f1a - if mode<0 || mode>1 : return; (wtf...)
	memcpy(hook_2p_setting_constraint,"\x83\xf8\x05",3); // => if mode<0 || mode>5

	char* hook_2p_setting = pe_data+0x200+0x3f27;
	memcpy(hook_2p_setting,"\xf\x85\xd3\xb0\x8\x0",6);

	/*
	hook rand seed at processCollisionBetweenBallAndPlayer 
	*/
	char* func_rand = pe_data+0x200+0x9320; //40a320
	char* new_func_rand_for_physical_collision = pe_data + func_start + 0x100; //490100
	memcpy(new_func_rand_for_physical_collision , func_rand , 0x2a); // copy rand() to 0x490100
	*(int*)(new_func_rand_for_physical_collision + 0x40a320-0x40a320+0x1) = 0x490200; // mov eax, [new_seed] (0x490200)
	*(int*)(new_func_rand_for_physical_collision + 0x40a33c-0x40a320+0x1) = 0x490200; // mov [new_seed], eax

	char *hook_collision_call_rand = pe_data+0x200+0x20d7+1; // 4030D7 , next instruc :4030DC
	*(int*) hook_collision_call_rand = 0x490100-0x4030DC;    // call 0x490100 (rew_rand)

	/*
	handle for game over
	e.g. mov dword ptr [esi+0C0h], 6  ==> "\xC7\x86\xC0\x00\x00\x00\x06\x00\x00\x00"
     =>  mov dword ptr [esi+0C0h], 0; nop ==> "\xc7\x86\xa4\x0\x0\x0\x0\x0\x0\x0\x90"
	len = 11
	*/
	char *hook_game_over_mode_set = pe_data + 0x200 + 0x134B; //0x40234B
	memcpy(hook_game_over_mode_set , "\xc7\x86\xa4\x0\x0\x0\x0\x0\x0\x0\x90", 11);


	//*(int*)(pe_data+0x1fc) = 0x60000060;
	FILE *out = fopen("./files/new_pika_IAT.exe","wb");
	if(out==NULL){
		printf("output failed\n");
	}
	fwrite(pe_data,1,pe_size,out);

	fclose(out);
	return 0;
}

int main(){
	expand();
	insert_func();
	modify_IID();
	return 0;
}