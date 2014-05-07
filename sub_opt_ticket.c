#include <stdio.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>

int main(){
	int sock = nn_socket (AF_SP, NN_SUB);

	char * val = "test.";
	char buf[10];
	size_t buf_sz = 10; 
	if (nn_setsockopt(sock, NN_SUB,NN_SUB_SUBSCRIBE, (void *) val, sizeof(val)))
		printf("Set Error %d: %s\n",nn_errno(), nn_strerror(nn_errno()));
	if(nn_getsockopt(sock, NN_SUB,NN_SUB_SUBSCRIBE, (void *) buf, &buf_sz))
		printf("Get Error %d: %s\n",nn_errno(), nn_strerror(nn_errno()));
	
	return 0;
}
