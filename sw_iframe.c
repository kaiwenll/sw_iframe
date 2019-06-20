#include "sw_iframe.h"

void sw_open_ts(void)
{
        FILE * fp;
        uint8_t buf[188];
	int i=0,ret=0,file_length;
        memset(buf,0,sizeof(buf));
	//printf("sizeof(buf)=%d\n",sizeof(buf));
        fp=fopen("./shoes.ts","rb");
	//获取文件长度
	fseek(fp,0,SEEK_END);
	file_length=ftell(fp);
	//将文件指针移动到文件首
	fseek(fp,0,SEEK_SET);
	sw_transport_packet_t *tspkt;
	//寻找PAT表
	for(;i<file_length/188;i++)
	{
		fread(buf,188,1,fp);
		ret=sw_ts_packet_parse(tspkt,buf);
		sw_show_ts_info(tspkt);
		if(ret==1)
			break;
		sw_reset_ts_struct(tspkt);
		
	}
	//找到PAT表，开始解析它
	sw_pat_parse(buf);
	ret=0;	
	fclose(fp);
}

//ts包的解析
int sw_ts_packet_parse(sw_transport_packet_t *ptspkt, uint8_t *data)
{
	uint8_t *ptr = data;
	ptspkt->sync_byte                    = ((ptr[0]));
	ptspkt->transport_error_indicator    = ((ptr[1]>>7));
	ptspkt->payload_unit_start_indicator = ((ptr[1]<<1) >> 7);
	ptspkt->transport_priority           = ((ptr[1]<<2) >> 7);
	ptspkt->pid                          = ((ptr[1]&0x1f)<<8) | ptr[2];
	ptspkt->transport_scrambling_control = ((ptr[3]&0xc0) >> 6);
	ptspkt->adaption_field_control       = ((ptr[3]&0x30) >> 4);
	ptspkt->continuity_counter           = ((ptr[3]&0x0f));
	 
	if(ptspkt->pid==0x0000)
		return 1;//返回1，说明是PAT表

	return 0;
}
//sw_transport_packet_t这个结构体各个变量复位
int sw_reset_ts_struct(sw_transport_packet_t *ptspkt)
{
	
	ptspkt->sync_byte                    = 0x00;
	ptspkt->transport_error_indicator    = 0x00;
	ptspkt->payload_unit_start_indicator = 0x00;
	ptspkt->transport_priority           = 0x00;
	ptspkt->pid                          = 0x00;
	ptspkt->transport_scrambling_control = 0x00;
	ptspkt->adaption_field_control       = 0x00;
	ptspkt->continuity_counter           = 0x00;

	return 0;
}
//pat表的解析，pid是0的ts包
int sw_pat_parse(uint8_t buf[])
{
	uint8_t *ptr=buf;
	

	
}

int sw_show_ts_info(sw_transport_packet_t *ptspkt)
{
	printf("|+++++++++++++++++++++++++++++++++++++++++++++|\n");
	printf("sizeof(transport_packet_t)   = %d  \n", (int)sizeof(sw_transport_packet_t));
	printf("sync_byte                    = 0x%x\n", ptspkt->sync_byte);           
	printf("transport_error_indicator    = 0x%x\n", ptspkt->transport_error_indicator); 
	printf("payload_unit_start_indicator = 0x%x\n", ptspkt->payload_unit_start_indicator);
	printf("transport_priority           = 0x%x\n", ptspkt->transport_priority);
	printf("pid                          = 0x%x\n", ptspkt->pid);
	printf("transport_scrambling_control = 0x%x\n", ptspkt->transport_scrambling_control);
	printf("adaption_field_control       = 0x%x\n", ptspkt->adaption_field_control);
	printf("continuity_counter           = 0x%x\n", ptspkt->continuity_counter);
	printf("adaptation_field_length      = 0x%x\n", ptspkt->adaptation_field_length);
	printf("pointer_field                = 0x%x\n", ptspkt->pointer_field);

	printf("playload_offset              = 0x%x\n", ptspkt->playload_offset);
	printf("|+++++++++++++++++++++++++++++++++++++++++++++|\n");
	return 0;
}

