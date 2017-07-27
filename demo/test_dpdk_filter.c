//create @20170512
#define _GNU_SOURCE
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "ip_filter.h"
#include "dpdk_driver.h"

//ÿ���̴߳�����ȡ���ݣ������﷢���ݶ���
typedef struct
{
	int rx_port_id;
	int rx_port_que_id;
	int tx_port_id;
	int tx_port_que_id;
	int thread_id;
}THREAD_PORT_INFO;

//�ϲ�ҵ�����ʱ��ز���
static int app_thread_cnt = 1;     //�ϲ㲢�д����߳���Ŀ
static int app_thread_coreid[] = {3,3,3,3}; //�ϲ�ÿ���̰߳󶨵ĺ�id
static THREAD_PORT_INFO thread_port_info[32] = {{0}};
	
//ip filter ����ʱ��ز���
static int ip_filter_capacity = 200000;  //ip filter��������ɵ�ip��Ŀ
static int ip_filter_cnt = 100000;       //����ʱ����ip filter����ip����Ŀ
IP_FILTER_TABLE filter_table;            //ȫ��filter���ɶ��߳�ʹ��

//����ͳ����Ϣ
typedef struct
{
	uint64_t recv_pkt;
	uint64_t drop_pkt;
	uint64_t match_pkt;
	uint64_t send_pkt;
}TEST_STAT;
static TEST_STAT test_stat[32];
static struct dpdk_init_para dpdk_para = {0};
//��ʼ��DPDK
int init_dpdk(void)
{
	if (0 > dpdk_nic_init("./dpdk.cfg", &dpdk_para))
	{
		printf("DPDK init Error! \n");
		return -1;
	}

	return 0;
}

//��ʼ��IP_FILTER
int init_filter(void)
{
	if (IP_FILTER_OK != ip_filter_init(ip_filter_capacity, &filter_table))
	{
		printf("IP Filter init Error, cnt:%d \n", ip_filter_capacity);
		return -1;
	}

	//���һЩ���ip����
	int i, ipv4Addr;
	for (i = 0; i < ip_filter_cnt - 1; i++)
	{
		//���һ�����������IP������У�����ѹ��
		ipv4Addr = (int32_t)random();
		if (IP_FILTER_OK != ip_filter_add(ipv4Addr, &filter_table))
		{
			printf("ID:%d ==========>Error====>Add ip:%d Error!\n", i, ipv4Addr);
			return -1;
		}
	}
		if (IP_FILTER_OK != ip_filter_add(3232286687, &filter_table))
	{
		printf("ID:%d ==========>Error====>Add ip:%d Error!\n", i, ipv4Addr);
		return -1;
	}

	//���������IP����
	ipv4Addr = 0x12345678;
	if (IP_FILTER_OK != ip_filter_add(ipv4Addr, &filter_table))
	{
		printf("==========>Error====>Add ip:%d Error!\n", ipv4Addr);
		return -1;
	}

	return 0;
}

int app_deal_something()
{
	int i,j,sum;
	for (i = 0; i < 200; i++)
		for (j = 0; j < 200; j++)
			sum += i*j;

	return sum;
}

static void* app_handle_thread(void* para)
{
	THREAD_PORT_INFO* thread_info = (THREAD_PORT_INFO*)para;
	int rx_port_id = thread_info->rx_port_id;
	int rx_port_que_id = thread_info->rx_port_que_id;
	int tx_port_id = thread_info->tx_port_id;
	int tx_port_que_id = thread_info->tx_port_que_id;
	int thread_id = thread_info->thread_id;
	int coreId = app_thread_coreid[thread_id];
	
	//���к��İ�
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(coreId, &mask);
	if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
	{
		printf("Handle ThreadID:%d bind to Core:%d Error !\n", thread_id, coreId);
		return NULL;
	}
	
	printf("Start Handle Thread, ThreadID:%d bind to Core:%d !\n", thread_id, coreId);

	//����ҵ����
	//1)�������ձ���;
	//2)���ýӿڽ���ƥ��;
	//3)���б��ĵ��߲�ҵ����;
	//4)͸�������ͷ�
	
	struct dpdk_pkt_info* recv_pkt = NULL;
	int32_t ipv4_recv;
	while (1)
	{
		//1)�������ձ���;
		if (0 > dpdk_recv_pkt(rx_port_id, rx_port_que_id, &recv_pkt))
		{
			usleep(10);
			continue; //�޿��ñ���
		}
		else
			test_stat[thread_id].recv_pkt++;

		//2)���ýӿڽ���ƥ��;
		if (IP_FILTER_OK != pkt_ip_match(recv_pkt->pkt_data,recv_pkt->pkt_len,&ipv4_recv,&filter_table))
		{
			//δƥ��ɹ��� ֱ���ͷ�
			//dpdk_drop_pkt(recv_pkt);
			//test_stat[thread_id].drop_pkt++;
			//continue;
		}
		else
		{
			test_stat[thread_id].match_pkt++;
		}

		//3)���б��ĵ��߲�ҵ����;
		app_deal_something();

		//4)͸�������ͷ�
		if (0 > dpdk_send_pkt(tx_port_id, tx_port_que_id, recv_pkt))
		{
			printf("Send Pkt Error!\n");
		}
		
		test_stat[thread_id].send_pkt++;

		usleep(10);
	}

	return NULL;
}

int init_app_thread()
{
	thread_port_info[0].rx_port_id = 0;
	thread_port_info[0].rx_port_que_id = 0;
	thread_port_info[0].tx_port_id = 0;
	thread_port_info[0].tx_port_que_id = 0;

	
	//thread_port_info[1].rx_port_id = 1;
	//thread_port_info[1].rx_port_que_id = 0;
	//thread_port_info[1].tx_port_id = 1;
	//thread_port_info[1].tx_port_que_id = 0;

	memset(test_stat, 0, sizeof(test_stat));
	int i;
	for (i = 0; i < app_thread_cnt; i++)
	{
		thread_port_info[i].thread_id = i;
		pthread_t threadID;
		if (0 != pthread_create(&threadID, NULL, app_handle_thread, &thread_port_info[i]))
		{
			printf("Create Thread ID:%d Error!\n", i);
			return -1;
		}
	}

	sleep(3);
	return 0;
}

void print_stat(void)
{
	int i = 0;
	printf("======================================================>>\n");
	for (; i < app_thread_cnt; i++)
	{
		printf("[ThreadID:%2d]Recv:%12lu    Drop:%12lu    Match:%12lu    Send:%12lu\n",i, \
			test_stat[i].recv_pkt, test_stat[i].drop_pkt, test_stat[i].match_pkt, test_stat[i].send_pkt);
	}
	printf("<<======================================================\n\n");

	unsigned portid;



	const char clr[] = { 27, '[', '2', 'J', '\0' };
	const char topLeft[] = { 27, '[', '1', ';', '1', 'H','\0' };

	struct dpdk_nic_port_statistics stats;
	

		/* Clear screen and move to top left */
	printf("%s%s", clr, topLeft);

	printf("\nPort statistics ====================================");

	for (portid = 0; portid < dpdk_para.port_num; portid++) {
		/* skip disabled ports */

		printf("\nStatistics for port %u ------------------------------\n",portid);
		dpdk_get_nic_port_statistics(portid, &stats);

		printf("RX-packets: %-12llu RX-bytes:  %-12llu\n", stats.rx_packets,stats.rx_bytes);
        printf("RX-error  : %-12llu Reason:[CRC or BadLen Error By Packet]\n",  stats.ierrors_packets);
	    printf("RX-nombuf : %-12llu Reason:[Mempool Not Enough, Not Equal To Packet Num]\n", stats.rx_nombuf);
        printf("RX-missed : %-12llu Reason:[Fwd Thread Been Scheduled or Enque Nic Que Error Happen]\n", stats.imissed_packets);
	    printf("TX-packets: %-12llu TX-bytes:  %-12llu TX-errors: %-12llu\n",
		   stats.tx_packets, stats.tx_bytes, stats.oerrors_packets);
	}

	dpdk_print_memory();
	return;
}

int main()
{
	if (0 > init_dpdk())    //��ʼ��dpdk
		return -1;
	
	if (0 > init_filter())  //��ʼ��ip filter ��
		return -1;

	if (0 > init_app_thread())  //��ӡͳ����Ϣ
		return -1;

	while(1)
	{
		print_stat(); //��ӡͳ����Ϣ
		sleep(5);
	}
	
	return 0;
}
