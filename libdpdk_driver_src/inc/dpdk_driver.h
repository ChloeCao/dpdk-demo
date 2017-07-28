#ifndef _DPDK_DRIVER_H_
#define _DPDK_DRIVER_H_
#include <stdint.h>

#define DPDK_MAX_PORT_NUM 32           /*dpdk�շ��ӿ�֧�ֵ����˿���*/
#define DPDK_MAX_CORE_NUM 64           /*dpdk�շ��ӿڿ����õ����cpu����*/
#define DPDK_MAX_APP_THREAD_NUM 64     /*�ϲ�Ӧ�ÿ����õ�����Ĵ����߳���*/
#define DPDK_MAX_QUE_NUM  32           /*ÿ����������ʹ�õ���������Ŀ*/


#define __dpdk_cache_aligned __attribute__((__aligned__(64)))



/* �����˿�״̬ͳ�� */
struct dpdk_nic_port_statistics {
	uint64_t rx_packets;        /*�����˿��հ�����*/
	uint64_t rx_bytes;          /*�����˿��հ��ֽ���*/
	uint64_t tx_packets;        /*�����˿ڷ�������*/
	uint64_t tx_bytes;          /*�����˿ڷ����ֽ���*/
	uint64_t imissed_packets;   /*�����˿���Ӳ�����������µĶ�������*/
	uint64_t ierrors_packets;   /*�����˿ڽ��ܴ��������*/
	uint64_t oerrors_packets;   /*�����˿ڷ��ʹ��������*/
	uint64_t rx_nombuf;         /*�����˿ڱ��Ļ������ʧ�ܴ���*/
} __dpdk_cache_aligned;


/* ҵ���߳��շ���״̬ͳ�� */
struct dpdk_app_thread_statistics {
	uint64_t rcv_packets;        /*ҵ���߳��հ�����*/
	uint64_t snd_packets;        /*ҵ���̷߳�������*/
	uint64_t drp_packets;        /*ҵ���̶߳�������*/
	uint64_t get_buffers;        /*ҵ���߳����뱨�Ļ���ɹ�����*/
	uint64_t get_nobuf;          /*ҵ���߳����뱨�Ļ���ʧ�ܴ���*/
} __dpdk_cache_aligned;



/*dpdk�շ��ӿڳ�ʼ����Ϣ�ṹ*/
struct dpdk_init_para{
    uint8_t  port_num;                                  /*���ö˿ڸ���*/
	uint8_t  core_num;                                  /*����CPU����*/
	uint8_t  core_arr[DPDK_MAX_CORE_NUM];               /*cpu������Ϣ����*/
}__dpdk_cache_aligned;


struct dpdk_port_conf{
	uint8_t  mode;									
	uint8_t  mac_addr[6];								/*�˿�mac��Ϣ*/
	uint8_t  rx_que_num;								/*�˿�ʹ�õ��ն�����Ŀ�������Ӧ���ϲ�Ӧ���߳���Ŀ���*/
    uint8_t  tx_que_num;                                /*�˿�ʹ�õķ�������Ŀ�������Ӧ���ϲ�Ӧ���߳���Ŀ���*/	
	uint8_t  rx_core_num;								/*�˿�ʹ�ü���cpu���Ľ����հ�*/
	uint8_t  rx_core_arr[DPDK_MAX_CORE_NUM];			/*cpu������Ϣ����*/
	uint8_t  tx_core_num;								/*�˿�ʹ�ü���cpu���Ľ��з���*/
	uint8_t  tx_core_arr[DPDK_MAX_CORE_NUM];			/*cpu������Ϣ����*/
	uint32_t cache_pkt_num;								/*�˿ڿ��Ի��汨�ĵĸ���*/
	void*	 mbuf_pool[DPDK_MAX_QUE_NUM];				/*���ڱ������հ��ڴ��*/
	void*    r_ring[DPDK_MAX_QUE_NUM];					/*���ڽ��յ��ı���MBUFָ�����ring�У���������һ��*/
	void*    t_ring[DPDK_MAX_QUE_NUM];					/*��������*/
}__dpdk_cache_aligned;

/*����������Ϣ�ṹ*/
struct dpdk_pkt_info {
    struct timeval time_stamp;              /*ʱ���*/
    uint8_t       *pkt_data;		        /*mac��ָ��*/
    uint16_t       pkt_len;                 /*����*/
    uint8_t        port_in;                 /*��˿�*/
    uint8_t        port_out;                /*���˿�*/
    uint8_t        core_id;                 /*�������*/
    uint8_t        drop_flag;               /*������־*/
    void           *pmbuf;                  /*mbufָ��*/
}__dpdk_cache_aligned;


/*********************************************************************************
* �������ƣ�dpdk_nic_init
* ����������dpdk�շ����ӿڳ�ʼ��������������Դ��
* ���룺
*      init_para���շ����ӿڳ�ʼ��������
* �������
* ����ֵ���ɹ�����0��ʧ�ܷ���-1��
*********************************************************************************/
int dpdk_nic_init(const char* file_path, struct dpdk_init_para* init_para);

/*********************************************************************************
* �������ƣ�dpdk_recv_pkt
* �����������հ��ӿڡ�
* ���룺
*      thread_id��  �߳���š�
* �����
*      recv_pkt�� ����������Ϣָ�롣
* ����ֵ���ɹ�����0��ʧ�ܷ���-1��
*********************************************************************************/
int	dpdk_recv_pkt(int port_id, int que_id,  struct dpdk_pkt_info **recv_pkt);


/*********************************************************************************
* �������ƣ�dpdk_send_pkt
* ���������������ӿڡ�
* ���룺
*      thread_id��  �߳���š�
*	   port_id:    ���Ͷ˿ڡ�
*      send_pkt�� ����������Ϣָ������
* �������
* ����ֵ����
*********************************************************************************/
int dpdk_send_pkt(int port_id, int que_id, struct dpdk_pkt_info *send_pkt);


/*********************************************************************************
* �������ƣ�dpdk_drop_pkt
* �����������ͷű��ġ�
* ���룺
*      thread_id��  �߳���š�
*	   ppkt�� ����������Ϣָ�롣
* �������
* ����ֵ����
*********************************************************************************/
void dpdk_drop_pkt(struct dpdk_pkt_info *ppkt);

/*********************************************************************************
* �������ƣ�dpdk_get_sendbuf
* ������������ȡ���Ļ���ӿڡ�
* ���룺
*      thread_id��  �߳���š�
* �����
*      ppkt�� ����������Ϣָ�롣
* ����ֵ���ɹ�����0��ʧ�ܷ���-1��
*********************************************************************************/
int dpdk_get_sendbuf(struct dpdk_pkt_info **ppkt);


/*********************************************************************************
* �������ƣ�dpdk_get_nic_port_statistics
* ������������ȡ������ָ���˿ڵı���ͳ��
* ���룺    port_id�� �˿ں�
* �����    nic_port_stats: ͳ��ֵ�ṹ��
* ����ֵ��  ��
*********************************************************************************/
void dpdk_get_nic_port_statistics(int port_id, struct dpdk_nic_port_statistics *nic_port_stats);


/*********************************************************************************
* �������ƣ�dpdk_app_thread_statistics
* ������������ȡָ��ҵ���̵߳ı���ͳ��
*           thread_id��  �߳���š�
* �����    app_thread_stats: ͳ��ֵ�ṹ��
* ����ֵ��  ��
*********************************************************************************/
//��δʵ��
void dpdk_get_app_thread_statistics(int thread_id, struct dpdk_app_thread_statistics *app_thread_stats);


/*********************************************************************************
* �������ƣ�dpdk_print_memory
* ����������ͳ�Ƶ�ǰ�ڴ�ص�ʹ�����
* �����    ��
* ����ֵ��  ��
*********************************************************************************/
void dpdk_print_memory(void);

#endif 
/* _DPDK_DRIVER_H_ */

