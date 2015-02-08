#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <openssl/md5.h>

#include "hs_netdef.h"
#include "hs_net_connector.h"
#include "hs_net_agent.h"

static const char* SERVER_IP = "192.168.2.70";
static const short SERVER_PORT = 10111;

static int my_conn_proc(void* param) {
	struct hs_net_connector* connector = (struct hs_net_connector*)param;
	if (connector) {
		printf("connector %d connected\n", connector->socket);
	}
	return 1;
}

static int my_disconn_proc(void* param) {
	int s = (int)(long)param;
	printf("connector %d disconnected\n", s);
	return 1;
}

static int my_data_proc(char* buf, int len, void* param) {
	char temp[4096];
	if (sizeof(temp)-1 < len)
		len = sizeof(temp)-1;
	memcpy(temp, buf, len);
	printf("get the data (%s)\n", temp);

	struct hs_net_connector* connector = (struct hs_net_connector*)param;
	hs_net_connector_write(connector, temp, len);
	return len;
}

int main(char** argc, int argv) {
	unsigned char* data = "123";
	unsigned char md[16];
	int i = 0;
	char tmp[3] = { '\0' }, buf[33] = { '\0' };
	MD5(data, strlen(data), md);
	for (; i<sizeof(md)/sizeof(md[0]); ++i) {
		sprintf(tmp, "%2.2x", md[i]);
		strcat(buf, tmp);
	}
	printf("%s\n", buf);

	struct hs_net_connector* hc = hs_net_connector_create(HS_DEFAULT_CONNECTOR_RECV_BUFF_LENGTH, HS_DEFAULT_CONNECTOR_SEND_BUFF_LENGTH, true);
	if (!hc) {
		printf("create connector failed\n");
		return -1;
	}

	hs_net_connector_data_handle(hc, my_data_proc);
	hs_net_connector_conn_handle(hc, my_conn_proc);
	hs_net_connector_disconn_handle(hc, my_disconn_proc);

	if (!hs_net_connector_connect(hc, SERVER_IP, SERVER_PORT)) {
		printf("connector connect server(%s:%d) failed\n", SERVER_IP, SERVER_PORT);
		return -1;
	}

	printf("connector start\n");
	
	int res = 0;
	while (1) {
		res = hs_net_connector_run(hc);
		if (res < 0) {
			printf("hs_server_run: server循环失败\n");
			break;
		}
		usleep(1000);
	}

	hs_net_connector_destroy(hc);

	return res;
}
